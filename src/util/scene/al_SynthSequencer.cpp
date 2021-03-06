
#include "al/util/scene/al_SynthSequencer.hpp"

using namespace al;

void SynthSequencer::render(AudioIOData &io) {
    if (mMasterMode ==  PolySynth::TIME_MASTER_AUDIO) {
        double timeIncrement = mNormalizedTempo * io.framesPerBuffer()/(double) io.framesPerSecond();
        double blockStartTime = mMasterTime;
        mMasterTime += timeIncrement;
        processEvents(blockStartTime, mNormalizedTempo * io.framesPerSecond());
    }
    mPolySynth->render(io);
}

void SynthSequencer::render(Graphics &g) {
    if (mMasterMode == PolySynth::TIME_MASTER_GRAPHICS) {
        double timeIncrement = 1.0/mFps;
        double blockStartTime = mMasterTime;
        mMasterTime += timeIncrement;
        processEvents(blockStartTime, mNormalizedTempo * mFps);
    }
    mPolySynth->render(g);
}

bool SynthSequencer::playSequence(std::string sequenceName, float startTime) {
  //        synth().allNotesOff();
  // Add an offset of 0.1 to make sure the allNotesOff message gets processed before the sequence
  double currentMasterTime = mMasterTime;
  const double startPad = 0.1;
  std::list<SynthSequencerEvent> events = loadSequence(sequenceName, currentMasterTime - startTime + startPad);

  std::unique_lock<std::mutex> lk(mEventLock);
  mEvents = events;
  mNextEvent = 0;
  mPlaybackStartTime = currentMasterTime;

  if (mMasterMode == PolySynth::TIME_MASTER_CPU) {
    mCpuThread = std::make_shared<std::thread>([&](int granularityns = 1000) {
      bool running = true;
      auto startTime = std::chrono::high_resolution_clock::now();
      while (running) {
        std::unique_lock<std::mutex> lk(mEventLock);
        if (mEvents.size() == 0) {
          running = false;
          if (verbose()) {
            std::cout << "CPU play thread done." << std::endl;
          }
        }
        lk.unlock();
        double timeIncrement = granularityns/ 1.0e-9;
        double blockStartTime = mMasterTime;
        mMasterTime += timeIncrement;
        processEvents(blockStartTime, 1.0e9/granularityns);
        std::this_thread::sleep_until(startTime + std::chrono::nanoseconds(uint32_t(mMasterTime * 1.0e9)));
      }
    });
  }
  return true;
}

void SynthSequencer::stopSequence() {
  std::unique_lock<std::mutex> lk(mEventLock);
  for (auto &event: mEvents) {
    if (event.type == SynthSequencerEvent::EVENT_VOICE) {
      // Give back allocated voice to synth
      mPolySynth->insertFreeVoice(event.voice);
    }
  }

  mEvents.clear();
  mNextEvent = 0;
}

void SynthSequencer::registerTimeChangeCallback(std::function<void (float)> func, float minTimeDeltaSec)
{
  if (mEventLock.try_lock()) {
    mTimeChangeCallback = func;
    mTimeChangeMinTimeDelta = minTimeDeltaSec;
    mEventLock.unlock();
  } else {
    std::cerr << "ERROR: Failed to set time change callback. Sequencer running" <<std::endl;
  }
}

std::string SynthSequencer::buildFullPath(std::string sequenceName)
{
  std::string fullName = mDirectory;

  if (fullName.back() != '/') {
    fullName += "/";
  }
  if (sequenceName.size() < 14 || sequenceName.substr(sequenceName.size() - 14) != ".synthSequence") {
    sequenceName += ".synthSequence";
  }
  fullName += sequenceName;
  return fullName;
}

std::list<SynthSequencerEvent> SynthSequencer::loadSequence(std::string sequenceName, double timeOffset, double timeScale) {
  std::unique_lock<std::mutex> lk(mLoadingLock);
  std::list<SynthSequencerEvent> events;
  std::string fullName = buildFullPath(sequenceName);
  std::ifstream f(fullName);
  if (!f.is_open()) {
    std::cout << "Could not open:" << fullName << std::endl;
    return events;
  }

  std::string line;
  double tempoFactor = 1.0;
  while (getline(f, line)) {
    if (line.substr(0, 2) == "::") {
      break;
    }
    std::stringstream ss(line);
    int command = ss.get();
    if (command == '@' && ss.get() == ' ') {
      std::string name, start, durationText;
      while (start.size() == 0) {
        std::getline(ss, start, ' ');
      }
      while (durationText.size() == 0) {
        std::getline(ss, durationText, ' ');
      }
      while (name.size() == 0) {
        std::getline(ss, name, ' ');
      }

      double startTime = std::stod(start) * timeScale * tempoFactor;
      double duration = std::stod(durationText) * timeScale * tempoFactor;

      // const int maxPFields = 64;
      std::vector<ParameterField> pFields;

      // int numFields = 0;
      std::string fieldsString;

      std::getline(ss, fieldsString);
      bool processingString = false;
      std::string stringAccum;
      size_t currentIndex = 0;

      auto isFloat = []( std::string myString ) {
        std::istringstream iss(myString);
        float f;
        iss >> std::noskipws >> f; // noskipws considers leading whitespace invalid
        // Check the entire string was consumed and if either failbit or badbit is set
        return iss.eof() && !iss.fail();
      };

      while(currentIndex < fieldsString.size()) {
        if (fieldsString[currentIndex] == '"') {
          if (processingString) { // String end
            pFields.push_back(stringAccum);
            stringAccum.clear();
            processingString = false;
          } else { // String begin
            processingString = true;
          }
        } else if (fieldsString[currentIndex] == ' '
                   || fieldsString[currentIndex] == '\t'
                   || fieldsString[currentIndex] == '\n'
                   || fieldsString[currentIndex] == '\r'
                   ) {
          if (processingString) {
            stringAccum += fieldsString[currentIndex];
          } else if (stringAccum.size() > 0) { // accumulate
            if (isFloat(stringAccum)) {
              pFields.push_back(stof(stringAccum));
            } else {
              pFields.push_back(stringAccum);
            }
            //                            std::cout << stringAccum << std::endl;
            stringAccum.clear();
          }
        } else { // Accumulate character
          stringAccum += fieldsString[currentIndex];
        }
        currentIndex++;
      }
      if (stringAccum.size() > 0) {
        if (isFloat(stringAccum)) {
          pFields.push_back(stof(stringAccum));
        } else {
          pFields.push_back(stringAccum);
        }
      }


      if (false) { // Insert event as EVENT_VOICE. This is not good as it forces preallocating all the events...
        SynthVoice *newVoice = mPolySynth->getVoice(name);
        if (newVoice) {
          if (!newVoice->setTriggerParams(pFields)) {
            std::cout << "Error setting pFields for voice of type " << name << ". Fields: ";
            for (auto &field: pFields) {
              std::cout << field.type() << " ";
            }
            std::cout << std::endl;
            mPolySynth->insertFreeVoice(newVoice); // Return voice to sequencer.
          } else {
            double absoluteTime = timeOffset + startTime;
            // Insert into event list, sorted.
            auto position = events.begin();
            while(position != events.end() && position->startTime < absoluteTime) {
              position++;
            }
            auto insertedEvent = events.insert(position, SynthSequencerEvent());
            // Add 0.1 padding to ensure all events play.
            insertedEvent->type = SynthSequencerEvent::EVENT_VOICE;
            insertedEvent->startTime = absoluteTime;
            insertedEvent->duration = duration;
            insertedEvent->voice = newVoice;
            //                        std::cout << "Inserted event " << events.size() << " at time " << absoluteTime << std::endl;
          }
        }
      } else {
        double absoluteTime = timeOffset + startTime;
        // Insert into event list, sorted.
        auto position = events.begin();
        while(position != events.end() && position->startTime < absoluteTime) {
          position++;
        }
        auto insertedEvent = events.insert(position, SynthSequencerEvent());
        // Add 0.1 padding to ensure all events play.
        insertedEvent->type = SynthSequencerEvent::EVENT_PFIELDS;
        insertedEvent->startTime = absoluteTime;
        insertedEvent->duration = duration;
        insertedEvent->fields.name = name;
        insertedEvent->fields.pFields = pFields; // TODO it would be nice not to have to copy here...
      }

      //                std::cout << "Done reading sequence" << std::endl;
    } else if (command == '+' && ss.get() == ' ') {
      std::string name, idText, start;
      std::getline(ss, start, ' ');
      std::getline(ss, idText, ' ');
      std::getline(ss, name, ' ');

      float startTime = std::stof(start) * timeScale * tempoFactor;
      int id = std::stoi(idText);
      const int maxPFields = 64;
      float pFields[maxPFields];

      int numFields = 0;
      std::string field;
      std::getline(ss, field, ' ');
      while (field != "" && numFields < maxPFields) {
        pFields[numFields] = std::stof(field);
        numFields++;
        std::getline(ss, field, ' ');
      }
      //                    std::cout << "Pfields: ";
      //                    for (int i = 0; i < numFields; i++) {
      //                        std::cout << pFields[i] << " ";
      //                    }
      //                    std::cout << std::endl;

      SynthVoice *newVoice = mPolySynth->getVoice(name);
      if (newVoice) {
        newVoice->id(id);
        if (!newVoice->setTriggerParams(pFields, numFields)) {
          std::cerr << "Error setting pFields for voice of type " << name << ". Fields: ";
          for (int i = 0; i < numFields; i++) {
            std::cerr << pFields[i] << " ";
          }
          std::cerr << std::endl;
        } else {
          std::list<SynthSequencerEvent>::iterator insertedEvent;
          double absoluteTime = timeOffset + startTime;
          {
            //                            // Insert into event list, sorted.
            auto position = events.begin();
            while(position != events.end() && position->startTime < absoluteTime) {
              position++;
            }
            insertedEvent = events.insert(position, SynthSequencerEvent());
          }
          // Add 0.1 padding to ensure all events play.
          insertedEvent->startTime = absoluteTime;
          insertedEvent->duration = -1; // Turn on events have undetermined duration until a turn off is found later
          insertedEvent->voice = newVoice;
          //                        std::cout << "Inserted event " << id << " at time " << startTime << std::endl;
        }
      } else {
        if (verbose()) {
          std::cout << "Warning: Unable to get free voice from PolySynth." << std::endl;
        }
      }
    } else if (command == '-' && ss.get() == ' ') {
      std::string time, idText;
      std::getline(ss, time, ' ');
      std::getline(ss, idText);
      int id = std::stoi(idText);
      double eventTime = std::stod(time) * timeScale * tempoFactor;
      for (SynthSequencerEvent &event: events) {
        if (event.voice->id() == id && event.duration < 0) {
          double duration = eventTime - event.startTime + timeOffset;
          if (duration < 0) {
            duration = 0;
          }
          event.duration = duration;
          //                        std::cout << "Set event duration " << id << " to " << duration << std::endl;
          break;
        }
      }
    } else if (command == '=' && ss.get() == ' ') {
      std::string time, sequenceName, timeScaleInFile;
      std::getline(ss, time, ' ');
      std::getline(ss, sequenceName, ' ');
      std::getline(ss, timeScaleInFile);
      if (sequenceName.at(0) == '"') {
        sequenceName = sequenceName.substr(1);
      }
      if (sequenceName.back() == '"') {
        sequenceName = sequenceName.substr(0, sequenceName.size() - 1);
      }
      lk.unlock();
      auto newEvents = loadSequence(sequenceName, stod(time) + timeOffset, stod(timeScaleInFile) * tempoFactor);
      lk.lock();
      events.insert(events.end(), newEvents.begin(), newEvents.end());
      // FIXME: This sorting only works if both the existing sequence and
      // the incoming sequence use absolute event times. Sorting
      // anything else results in chaos... This should be detected
      // and acted on
      events.sort([](const SynthSequencerEvent &a, const SynthSequencerEvent &b) {
        return a.startTime < b.startTime;
      });
    } else if (command == '>' && ss.get() == ' ') {
      std::string time, sequenceName;
      std::getline(ss, time);
      timeOffset += std::stod(time);
    } else if (command == 't' && ss.get() == ' ') {
      std::string tempo;
      std::getline(ss, tempo);
      tempoFactor = 60.0/std::stod(tempo);
    } else {
      if (command > 0) {
        if(verbose()) {
          std::cout << "Line ignored. Command: " << command << std::endl;
        }
      }
    }
  }
  f.close();
  if (f.bad()) {
    std::cout << "Error reading:" << fullName << std::endl;
  }
  return events;
}

std::vector<std::string> SynthSequencer::getSequenceList() {
  std::vector<std::string> sequenceList;
  std::string path = mDirectory;
  if (!File::isDirectory(path)) {
    Dir::make(path);
  }

  // get list of files ending in ".synthSequence"
  FileList sequence_files = filterInDir(path, [](const FilePath& f){
    if (al::checkExtension(f, ".synthSequence")) return true;
    else return false;
  });

  // store found preset files
  for (int i = 0; i < sequence_files.count(); i += 1) {
    const FilePath& path = sequence_files[i];
    const std::string& name = path.file();
    // exclude extension when adding to sequence list
    sequenceList.push_back(name.substr(0, name.size()-14));
  }


  std::sort(sequenceList.begin(), sequenceList.end(), [](const auto& lhs, const auto& rhs){
    const auto result = mismatch(lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend(), [](const auto& lhs, const auto& rhs){return tolower(lhs) == tolower(rhs);});

    return result.second != rhs.cend() && (result.first == lhs.cend() || tolower(*result.first) < tolower(*result.second));
  });

  return sequenceList;
}

double SynthSequencer::getSequenceDuration(std::string sequenceName) {
  std::list<SynthSequencerEvent> events = loadSequence(sequenceName, 0.0);
  double dur = 0.0;
  for (auto const &event: events) {
    if (event.startTime + event.duration > dur) {
      dur = event.startTime + event.duration;
    }
  }
  return dur;
}

void SynthSequencer::processEvents(double blockStartTime, double fpsAdjusted) {
  if (mEventLock.try_lock()) {

    if (mNextEvent < mEvents.size()) {

      mTimeAccumCallbackNs += (mMasterTime - blockStartTime)* 1.0e9;
      if (mTimeAccumCallbackNs*1.0e-9 > mTimeChangeMinTimeDelta) {
        if (mTimeChangeCallback) {
          mTimeChangeCallback(float(blockStartTime - mPlaybackStartTime));
//          std::cout << blockStartTime- mPlaybackStartTime << std::endl;
        }
        mTimeAccumCallbackNs -= mTimeChangeMinTimeDelta* 1.0e9;
      }
      auto iter = mEvents.begin();
      std::advance(iter, mNextEvent);
      auto event = iter;
      while (event->startTime <= mMasterTime) {
        event->offsetCounter = (event->startTime - blockStartTime)*fpsAdjusted;
        if (event->type == SynthSequencerEvent::EVENT_VOICE) {
          mPolySynth->triggerOn(event->voice, event->offsetCounter);
          event->voice = nullptr; // Voice has been consumed
        } else if (event->type == SynthSequencerEvent::EVENT_PFIELDS){
          auto *voice = mPolySynth->getVoice(event->fields.name);
          if (voice) {
            voice->setTriggerParams(event->fields.pFields);
            mPolySynth->triggerOn(voice, event->offsetCounter);
            event->voice = voice;
          } else {
            std::cerr << "SynthSequencer::processEvents: Could not get free voice for sequencer!" << std::endl;
          }
        } else if (event->type == SynthSequencerEvent::EVENT_TEMPO){
          // TODO support tempo events
        }
        //                std::cout << "Event " << mNextEvent << " " << event->startTime << " " << typeid(*event->voice).name() << std::endl;
        mNextEvent++;
        iter++;
        if (iter == mEvents.end()) {
          break;
        }
        event = iter;
      }
    }
    for (auto &event : mEvents) {
      //            if (event.startTime*mNormalizedTempo > mMasterTime) {
      //                break;
      //            }
      double eventTermination = event.startTime + event.duration;
      if (event.voice && event.voice->active() && eventTermination <= mMasterTime) {
        mPolySynth->triggerOff(event.voice->id());
        //                std::cout << "trigger off " <<  event.voice->id() << " " << eventTermination << " " << mMasterTime  << std::endl;
        event.voice = nullptr; // When an event gives up a voice, it is done.
      }
    }
    mEventLock.unlock();
  }
}

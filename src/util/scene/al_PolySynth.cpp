#include <memory>

#include "al/util/scene/al_PolySynth.hpp"

using namespace al;

#ifdef __GNUG__
#include <cstdlib>
#include <memory>
#include <cxxabi.h>

std::string al::demangle(const char* name) {

    int status = -4; // some arbitrary value to eliminate the compiler warning

    // enable c++11 by passing the flag -std=c++11 to g++
    std::unique_ptr<char, void(*)(void*)> res {
        abi::__cxa_demangle(name, NULL, NULL, &status),
        std::free
    };

    return (status==0) ? res.get() : name ;
}

#else

#if AL_WINDOWS

// does nothing if not g++
std::string al::demangle(const char* name) {
    // Windows prepends "class " here, so remove it
    return std::string(name).substr(6);
}

#else
// does nothing if not g++
std::string al::demangle(const char* name) {
    return name;
}
#endif

#endif

int al::asciiToIndex(int asciiKey, int offset) {
    switch(asciiKey){
    case '1': return offset + 0;
    case '2': return offset + 1;
    case '3': return offset + 2;
    case '4': return offset + 3;
    case '5': return offset + 4;
    case '6': return offset + 5;
    case '7': return offset + 6;
    case '8': return offset + 7;
    case '9': return offset + 8;
    case '0': return offset + 9;

    case 'q': return offset + 10;
    case 'w': return offset + 11;
    case 'e': return offset + 12;
    case 'r': return offset + 13;
    case 't': return offset + 14;
    case 'y': return offset + 15;
    case 'u': return offset + 16;
    case 'i': return offset + 17;
    case 'o': return offset + 18;
    case 'p': return offset + 19;
    case 'a': return offset + 20;

    case 's': return offset + 21;
    case 'd': return offset + 22;
    case 'f': return offset + 23;
    case 'g': return offset + 24;
    case 'h': return offset + 25;
    case 'j': return offset + 26;
    case 'k': return offset + 27;
    case 'l': return offset + 28;
    case ';': return offset + 29;

    case 'z': return offset + 30;
    case 'x': return offset + 31;
    case 'c': return offset + 32;
    case 'v': return offset + 33;
    case 'b': return offset + 34;
    case 'n': return offset + 35;
    case 'm': return offset + 36;
    case ',': return offset + 37;
    case '.': return offset + 38;
    case '/': return offset + 39;
    }
    return 0;
}

int al::asciiToMIDI(int asciiKey, int offset) {
    switch(asciiKey){
    //	case '1': return offset + 0;
    case '2': return offset + 73;
    case '3': return offset + 75;
        //	case '4': return offset + 3;
    case '5': return offset + 78;
    case '6': return offset + 80;
    case '7': return offset + 82;
        //	case '8': return offset + 7;
    case '9': return offset + 85;
    case '0': return offset + 87;

    case 'q': return offset + 72;
    case 'w': return offset + 74;
    case 'e': return offset + 76;
    case 'r': return offset + 77;
    case 't': return offset + 79;
    case 'y': return offset + 81;
    case 'u': return offset + 83;
    case 'i': return offset + 84;
    case 'o': return offset + 86;
    case 'p': return offset + 88;

        //	case 'a': return offset + 20;
    case 's': return offset + 61;
    case 'd': return offset + 63;
        //	case 'f': return offset + 23;
    case 'g': return offset + 66;
    case 'h': return offset + 68;
    case 'j': return offset + 70;
        //	case 'k': return offset + 27;
    case 'l': return offset + 73;
    case ';': return offset + 75;

    case 'z': return offset + 60;
    case 'x': return offset + 62;
    case 'c': return offset + 64;
    case 'v': return offset + 65;
    case 'b': return offset + 67;
    case 'n': return offset + 69;
    case 'm': return offset + 71;
    case ',': return offset + 72;
    case '.': return offset + 74;
    case '/': return offset + 76;
    }
    return 0;
}


// ----------------------------

int PolySynth::triggerOn(SynthVoice *voice, int offsetFrames, int id, void *userData) {
    assert(voice);
    int thisId = id;
    if (thisId == -1) {
        if (voice->id() > 0) {
            thisId = voice->id();
        } else {
            thisId = mIdCounter++;
        }
    }
    voice->id(thisId);
    if (userData) {
        voice->userData(userData);
    }
    voice->triggerOn(offsetFrames);
    {
        std::unique_lock<std::mutex> lk(mVoiceToInsertLock);
        voice->next = mVoicesToInsert;
        mVoicesToInsert = voice;
    }
    for (auto cbNode: mTriggerOnCallbacks) {
        cbNode.first(voice, offsetFrames, thisId, cbNode.second);
    }
    return thisId;
}

void PolySynth::triggerOff(int id) {
    mVoiceIdsToTurnOff.write((const char*) &id, sizeof (int));
    for (auto cbNode: mTriggerOffCallbacks) {
        cbNode.first(id, cbNode.second);
    }
}

void PolySynth::allNotesOff()
{
    mAllNotesOff = true;
}

SynthVoice *PolySynth::getVoice(std::string name, bool forceAlloc)
{
    std::unique_lock<std::mutex> lk(mFreeVoiceLock); // Only one getVoice() call at a time
    SynthVoice *freeVoice = mFreeVoices;
    SynthVoice *previousVoice = nullptr;
    while (freeVoice) {
      if (verbose()) {
        std::cout << "Comparing  voice '" << demangle(typeid(*freeVoice).name()) << "' to '" << name << "'" << std::endl;
      }
        if (demangle(typeid(*freeVoice).name()) == name
                || strncmp(typeid(*freeVoice).name(),name.c_str(), name.size()) == 0 ) {

            if (previousVoice) {
                previousVoice->next = freeVoice->next;
            } else {
                mFreeVoices = freeVoice->next;
            }
            break;
        }
        previousVoice = freeVoice;
        freeVoice = freeVoice->next;
    }
    if (!freeVoice) { // No free voice in list, so we need to allocate it
    //  But only allocate if allocation has not been disabled
        if (std::find(mNoAllocationList.begin(), mNoAllocationList.end(), name) == mNoAllocationList.end()) {
             // TODO report current polyphony for more informed allocation of polyphony
            freeVoice = allocateVoice(name);
        } else {
            std::cout << "Automatic allocation disabled for voice:" << name << std::endl;
        }

    }
    return freeVoice;
}

void PolySynth::render(AudioIOData &io) {
    if (mMasterMode == TIME_MASTER_AUDIO) {
        processVoices();
        // Turn off voices
        processVoiceTurnOff();
    }

    // Render active voices
    auto voice = mActiveVoices;
    int fpb = io.framesPerBuffer();
    while (voice) {
        if (voice->active()) {
            int offset = voice->getStartOffsetFrames(fpb);
            if (offset < fpb) {
                io.frame(offset);
                int endOffsetFrames = voice->getEndOffsetFrames(fpb);
                if (endOffsetFrames > 0 && endOffsetFrames <= fpb) {
                    voice->triggerOff(endOffsetFrames);
                }
                voice->onProcess(io);
            }
        }
        voice = voice->next;
    }
    processGain(io);

    // Run post processing callbacks
    for (auto cb: mPostProcessing) {
        io.frame(0);
        cb->onAudioCB(io);
    }
    if (mMasterMode == TIME_MASTER_AUDIO) {
        processInactiveVoices();
    }
}

void PolySynth::render(Graphics &g) {
    if (mMasterMode == TIME_MASTER_GRAPHICS) {
        processVoices();
        // Turn off voices
        processVoiceTurnOff();
    }
    std::unique_lock<std::mutex> lk(mGraphicsLock);
    SynthVoice *voice = mActiveVoices;
    while (voice) {
        // TODO implement offset?
        if (voice->active()) {
            voice->onProcess(g);
        }
        voice = voice->next;
    }
    if (mMasterMode == TIME_MASTER_GRAPHICS) {
        processInactiveVoices();
    }
}

void PolySynth::update(double dt) {
    if (mMasterMode == TIME_MASTER_ASYNC) {
        processVoices();
        // Turn off voices
        processVoiceTurnOff();
    }
    std::unique_lock<std::mutex> lk(mGraphicsLock);
    SynthVoice *voice = mActiveVoices;
    while (voice) {
        if (voice->active()) {
            voice->update(dt);
        }
        voice = voice->next;
    }
    if (mMasterMode == TIME_MASTER_ASYNC) {
        processInactiveVoices();
    }
}

void PolySynth::allocatePolyphony(std::string name, int number)
{
    std::unique_lock<std::mutex> lk(mFreeVoiceLock);
    // Find last voice and add polyphony there
    SynthVoice *lastVoice = mFreeVoices;
    if (lastVoice) {
        while (lastVoice->next) { lastVoice = lastVoice->next; }
    } else {
        lastVoice = mFreeVoices = allocateVoice(name);
        number--;
    }
    for(int i = 0; i < number; i++) {
        lastVoice->next = allocateVoice(name);
        lastVoice = lastVoice->next;
    }
}

void PolySynth::insertFreeVoice(SynthVoice *voice) {
    std::unique_lock<std::mutex> lk(mFreeVoiceLock);
    SynthVoice *lastVoice = mFreeVoices;
    if (lastVoice) {
        while (lastVoice->next) { lastVoice = lastVoice->next; }
        lastVoice->next = voice;
        voice->next = nullptr;
    } else {
        mFreeVoices = voice;
        voice->next = nullptr;
    }
}

bool PolySynth::popFreeVoice(SynthVoice *voice) {
    std::unique_lock<std::mutex> lk(mFreeVoiceLock);
    SynthVoice *lastVoice = mFreeVoices;
    SynthVoice *previousVoice = nullptr;
    while (lastVoice) {
        if (lastVoice == voice) {
            if (previousVoice) {
                previousVoice->next = lastVoice->next;
                voice->next = nullptr;
            } else {
                mFreeVoices = lastVoice->next;
                voice->next = nullptr;
            }
            return true;
        }
        lastVoice = lastVoice->next;
    }
    return false;
}

PolySynth &PolySynth::append(AudioCallback &v) {
    mPostProcessing.push_back(&v);
    return *this;
}

PolySynth &PolySynth::prepend(AudioCallback &v) {
    mPostProcessing.insert(mPostProcessing.begin(), &v);
    return *this;
}

PolySynth &PolySynth::insertBefore(AudioCallback &v, AudioCallback &beforeThis) {
    std::vector<AudioCallback *>::iterator pos =
            std::find(mPostProcessing.begin(), mPostProcessing.end(), &beforeThis);
    if (pos == mPostProcessing.begin()) {
        prepend(v);
    } else {
        mPostProcessing.insert(--pos, &v);
    }
    return *this;
}

PolySynth &PolySynth::insertAfter(AudioCallback &v, AudioCallback &afterThis) {
    std::vector<AudioCallback *>::iterator pos =
            std::find(mPostProcessing.begin(), mPostProcessing.end(), &afterThis);
    if (pos == mPostProcessing.end()) {
        append(v);
    } else {
        mPostProcessing.insert(pos, &v);
    }
    return *this;
}

PolySynth &PolySynth::remove(AudioCallback &v) {
    // the proper way to do it:
    mPostProcessing.erase(
                std::remove(mPostProcessing.begin(), mPostProcessing.end(), &v),
                mPostProcessing.end());
    return *this;
}

void PolySynth::print(std::ostream &stream) {
    {
        std::unique_lock<std::mutex> lk(mFreeVoiceLock);
        auto voice = mFreeVoices;
        int counter = 0;
        stream << " ---- Free Voices ----" << std:: endl;
        while(voice) {
            stream << "Voice " << counter++ << " " << voice->id() << " : " <<  typeid(voice).name() << " " << voice << std::endl;
            voice = voice->next;
        }
    }
    //
    {
        auto voice = mActiveVoices;
        int counter = 0;
        stream << " ---- Active Voices ----" << std:: endl;
        while(voice) {
            stream << "Voice " << counter++ << " " << voice->id() << " : " <<  typeid(voice).name() << " " << voice  << std::endl;
            voice = voice->next;
        }
    }
    //
    {
        std::unique_lock<std::mutex> lk(mVoiceToInsertLock);
        auto voice = mVoicesToInsert;
        int counter = 0;
        stream << " ---- Queued Voices ----" << std:: endl;
        while(voice) {
            stream << "Voice " << counter++ << " " << voice->id() << " : " <<  typeid(voice).name() << " " << voice  << std::endl;
            voice = voice->next;
        }
    }
}


int SynthVoice::getStartOffsetFrames(unsigned int framesPerBuffer) {
  int frames = mOnOffsetFrames;
  mOnOffsetFrames -= framesPerBuffer;
  if (mOnOffsetFrames < 0) {mOnOffsetFrames = 0;}
  return frames;
}

int SynthVoice::getEndOffsetFrames(unsigned int framesPerBuffer) {
  int frames = mOffOffsetFrames;
  mOffOffsetFrames -= framesPerBuffer;
  if (mOffOffsetFrames < 0) {mOffOffsetFrames = 0;}
  return frames;
}

#include "al_ext/soundfile/al_SoundfileBuffered.hpp"

using namespace al;

SoundFileBuffered::SoundFileBuffered(std::string fullPath, bool loop, int bufferFrames) :
  mRunning(true),
  mLoop(loop),
  mRepeats(0),
  mSeek(-1),
  mCurPos(0),
  mBufferFrames(bufferFrames),
  mReadCallback(nullptr)
{
  open(fullPath);
}

SoundFileBuffered::~SoundFileBuffered()
{
  close();
}

bool SoundFileBuffered::open(std::string fullPath)
{
  close();
  mSf.path(fullPath);
  mSf.openRead();
  if (mSf.opened()) {
    mRingBuffer = new SingleRWRingBuffer(mBufferFrames * channels() * sizeof(float));
    mReaderThread = new std::thread(readFunction, this);
    mFileBuffer = new float[mBufferFrames * channels()];
    return true;
  }
  return false;
}

bool SoundFileBuffered::close()
{

  if (mSf.opened()) {
    mRunning = false;
    mCondVar.notify_one();
    mReaderThread->join();
    delete mReaderThread;
    delete mRingBuffer;
    delete[] mFileBuffer;
    mSf.close();
  }
  return true;
}

size_t SoundFileBuffered::read(float *buffer, int numFrames)
{
  size_t bytesRead = mRingBuffer->read((char *) buffer, numFrames * channels() * sizeof(float));
  if (bytesRead != numFrames * channels() * sizeof(float)) {
    // TODO: handle underrun
  }
  mCondVar.notify_one();
  return bytesRead / (channels() * sizeof(float));
}

bool SoundFileBuffered::opened() const
{
  return mSf.opened();
}

void SoundFileBuffered::readFunction(SoundFileBuffered  *obj)
{
  while (obj->mRunning) {
    std::unique_lock<std::mutex> lk(obj->mLock);
    obj->mCondVar.wait(lk);
    int framesToRead = obj->mRingBuffer->writeSpace() / (obj->channels() * sizeof(float));
    int framesRead = obj->mSf.read(obj->mFileBuffer, framesToRead);
    std::atomic_fetch_add(&(obj->mCurPos), framesRead);
    int seek = obj->mSeek.load();
    if (seek >= 0) { // Process seek request
      obj->mSf.seek(seek, SEEK_SET);
      obj->mSeek.store(-1);
    }
    if (framesRead != framesToRead) { // Final incomplete buffer in the file
      framesRead += obj->mSf.read(obj->mFileBuffer + framesRead, framesToRead - framesRead);
      if (obj->mLoop) {
        obj->mSf.seek(0, SEEK_SET);
        std::atomic_fetch_add(&(obj->mRepeats), 1);
      }
    }
    size_t written = obj->mRingBuffer->write((const char*) obj->mFileBuffer, framesRead * sizeof(float) * obj->channels());
    //		if (written != framesRead * sizeof(float) * obj->channels()) {
    //			// TODO handle overrun
    //		}
    if (obj->mReadCallback) {
      obj->mReadCallback(obj->mFileBuffer, obj->mSf.channels(), framesRead, obj->mCallbackData);
    }
    lk.unlock();
  }
}

gam::SoundFile::EncodingType SoundFileBuffered::encoding() const
{
  if (opened()) {
    return mSf.encoding();
  } else {
    return (gam::SoundFile::EncodingType) 0;
  }
}

gam::SoundFile::Format SoundFileBuffered::format() const
{
  if (opened()) {
    return mSf.format();
  } else {
    return (gam::SoundFile::Format) 0;
  }
}

double SoundFileBuffered::frameRate() const
{
  if (opened()) {
    return mSf.frameRate();
  } else {
    return 0.0;
  }
}

int SoundFileBuffered::frames() const
{
  if (opened()) {
    return mSf.frames();
  } else {
    return 0;
  }
}

int SoundFileBuffered::channels() const
{
  if (opened()) {
    return mSf.channels();
  } else {
    return 0;
  }
}

int SoundFileBuffered::samples() const
{
  if (opened()) {
    return mSf.samples();
  } else {
    return 0;
  }
}

int SoundFileBuffered::repeats()
{
  return mRepeats.load();
}

void SoundFileBuffered::setReadCallback(SoundFileBuffered::CallbackFunc func, void *userData)
{
  mReadCallback = func;
  mCallbackData = userData;
}

void SoundFileBuffered::seek(int frame)
{
  if (frame < 0) {
    frame = 0;
  }
  if (frame >= frames()) {
    frame = frames() - 1;
  }
  mSeek.store(frame);
  mCurPos.store(frame);
}

int SoundFileBuffered::currentPosition()
{
  return mCurPos.load();
}

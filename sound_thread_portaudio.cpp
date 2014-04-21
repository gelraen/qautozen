#include "sound_thread_portaudio.h"
#include <QAtomicPointer>
#include <QDebug>
#include <QReadWriteLock>
#include <QReadLocker>
#include <QWriteLocker>
#include <vector>
#include <cmath>
#include <stdint.h>

using std::floor;
using std::pow;
using std::sin;
using std::vector;

const int kSampleRate = 8000;
typedef uint8_t sampleType;
const int kSampleSize = sizeof(sampleType);
const int kChannelCount = 2;  // stereo

int waveTable[kSampleRate] = {0};

void initWaveTable() {
  double increment = (2 * M_PI) / kSampleRate;
  double current = 0;
  for (int i = 0; i < kSampleRate; i++, current += increment) {
    waveTable[i] = floor(sin(current) * 127);
  }
}

struct CurrentState;

struct SoundThreadData {
  SoundThreadData() {
    for (int i = 0; i < kHarmonicsMax; i++) {
      harmonics_curtime_left_[i] = 0;
      harmonics_curtime_right_[i] = 0;
    }
  }
  double harmonics_curtime_left_[kHarmonicsMax];
  double harmonics_curtime_right_[kHarmonicsMax];
};

CurrentState *state = NULL;
QReadWriteLock stateMtx;

int adjustToRange(int v, int min, int max) {
  return v < min ? min : (v > max ? max : v);
}

struct CurrentState {
  double base_;
  double beat_;
  int harmonics_;
  int volume_;

 public:
  CurrentState()
      : base_(kBaseDefault),
        beat_(kBeatDefault),
        harmonics_(kHarmonicsDefault),
        volume_(kVolumeDefault) {}

  void IncrementCurtimes(SoundThreadData* d) const {
    for (int i = 0; i < kHarmonicsMax; i++) {
      d->harmonics_curtime_left_[i] =
          fmod(d->harmonics_curtime_left_[i] + base_ * pow(2, i), kSampleRate);
    }
    for (int i = 0; i < harmonics_; i++) {
      d->harmonics_curtime_right_[i] =
          fmod(d->harmonics_curtime_right_[i] + base_ * pow(2, i) + beat_,
               kSampleRate);
    }
  }

  uint8_t GetLeft(const SoundThreadData& d) const {
    int sigma = 0;
    for (int i = 0; i < harmonics_; i++) {
      sigma += waveTable[(int)floor(d.harmonics_curtime_left_[i])] >> i;
    }
    sigma = sigma / 2 + 128;
    return floor(volume_ * sigma / kVolumeMax);
  }

  uint8_t GetRight(const SoundThreadData& d) const {
    int sigma = 0;
    for (int i = 0; i < harmonics_; i++) {
      sigma += waveTable[(int)floor(d.harmonics_curtime_right_[i])] >> i;
    }
    sigma = sigma / 2 + 128;
    return floor(volume_ * sigma / kVolumeMax);
  }
};

int fillBuffer(const void* /* input */, void* output, unsigned long frameCount,
               const PaStreamCallbackTimeInfo* /* timeinfo */,
               PaStreamCallbackFlags /* statusFlags */, void* userData) {
  SoundThreadData* data = (SoundThreadData*)userData;
  QReadLocker locker(&stateMtx);
  sampleType* out = (sampleType*)output;
  for (unsigned long i = 0; i < frameCount; i++) {
    state->IncrementCurtimes(data);
    out[0] = state->GetLeft(*data);
    out[1] = state->GetRight(*data);
    out += 2 * sizeof(sampleType);
  }
  return paContinue;
}

void updateState(std::function<void(CurrentState*)> f) {
  QWriteLocker locker(&stateMtx);
  f(state);
}

SoundManager::SoundManager(QObject* parent) : QObject(parent), stream_(NULL) {
  PaError err = Pa_Initialize();
  if (err != paNoError) {
    qDebug() << "Failed to initialize portaudio: " << Pa_GetErrorText(err);
  }
  initWaveTable();
  initOut();
}

SoundManager::~SoundManager() {
  Pa_CloseStream(stream_);
  Pa_Terminate();
}

void SoundManager::initOut() {
  PaStream* stream;
  PaError err = Pa_OpenDefaultStream(&stream, 0, 2, paUInt8, kSampleRate, 0,
                                     fillBuffer, new SoundThreadData());
  if (err != paNoError) {
    qDebug() << "Failed to open a stream: " << Pa_GetErrorText(err);
    return;
  }
  QWriteLocker locker(&stateMtx);
  if (state != NULL) {
    delete state;
  }
  state = new CurrentState();
  stream_ = stream;
}

void SoundManager::start() {
  if (stream_ == NULL) {
    return;
  }
  PaError err = Pa_StartStream(stream_);
  if (err != paNoError) {
    qDebug() << "Failed to start stream: " << Pa_GetErrorText(err);
  }
  qDebug() << "start";
}

void SoundManager::stop() {
  if (stream_ == NULL) {
    return;
  }
  PaError err = Pa_StopStream(stream_);
  if (err != paNoError) {
    qDebug() << "Failed to stop stream: " << Pa_GetErrorText(err);
  }
  qDebug() << "stop";
}

void SoundManager::setBeat(int v) {
  v = adjustToRange(v, kBeatMin, kBeatMax);
  updateState([v](CurrentState* s) { s->beat_ = v; });
  emit beatChanged(v);
}

void SoundManager::setBase(int v) {
  v = adjustToRange(v, kBaseMin, kBaseMax);
  updateState([v](CurrentState* s) { s->base_ = v; });
  emit baseChanged(v);
}

void SoundManager::setVolume(int v) {
  v = adjustToRange(v, kVolumeMin, kVolumeMax);
  updateState([v](CurrentState* s) { s->volume_ = v; });
  emit volumeChanged(v);
}

void SoundManager::setHarmonics(int v) {
  v = adjustToRange(v, kHarmonicsMin, kHarmonicsMax);
  updateState([v](CurrentState* s) { s->harmonics_ = v; });
  emit harmonicsChanged(v);
}

int SoundManager::getBeat() {
  QReadLocker locker(&stateMtx);
  return state->beat_;
}

int SoundManager::getBase() {
  QReadLocker locker(&stateMtx);
  return state->base_;
}

int SoundManager::getVolume() {
  QReadLocker locker(&stateMtx);
  return state->volume_;
}

int SoundManager::getHarmonics() {
  QReadLocker locker(&stateMtx);
  return state->harmonics_;
}

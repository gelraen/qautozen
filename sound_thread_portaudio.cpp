#include "sound_thread_portaudio.h"
#include <QAtomicPointer>
#include <QDebug>
#include <vector>
#include <cmath>
#include <stdint.h>

using std::floor;
using std::pow;
using std::sin;
using std::vector;

const int kSampleRate = 8000;
const int kMaxHarmonics = 10;
const int kDefaultHarmonics = 3;
const int kBeatMax = 40;
typedef uint8_t sampleType;
const int kSampleSize = sizeof(sampleType);
const int kChannelCount = 2; // stereo
const int kBufferLenMs = 125;

struct CurrentState;

QAtomicPointer<CurrentState> state;

struct CurrentState {
private:
	vector<int> waveTable_;
	int sample_rate_;
	int harmonics_;
	vector<double> harmonic_curtime_left_;
	vector<double> harmonic_curtime_right_;
public:
	double base_;
	double beat_;
	int volume_;

public:
	CurrentState(int sampleRate) : sample_rate_(sampleRate), base_(300), beat_(20), volume_(100) {
		double increment = (2*M_PI) / sample_rate_;
		double current = 0;

		waveTable_ = vector<int>(sample_rate_);
		for(int i = 0; i < sample_rate_; i++, current += increment) {
			waveTable_[i] = floor( sin(current) * 127);
		}

		SetHarmonics(kDefaultHarmonics);
	}

	void SetHarmonics(int n) {
		harmonic_curtime_left_ = vector<double>(n);
		harmonic_curtime_right_ = vector<double>(n);
		harmonics_ = n;
	}

	void SetVolume(int v) {
		volume_ = v;
	}

	void IncerementCurtimes() {
		for(int i = 0; i < harmonics_; i++) {
			harmonic_curtime_left_[i] =
				fmod(harmonic_curtime_left_[i] + base_ * pow(2, i), sample_rate_);
		}
		for(int i = 0; i < harmonics_; i++) {
			harmonic_curtime_right_[i] =
				fmod(harmonic_curtime_right_[i] + base_ * pow(2, i) + beat_, sample_rate_);
		}
	}

	uint8_t GetLeft() {
		int sigma = 0;
		for(int i = 0; i < harmonics_; i++) {
			sigma += waveTable_[(int)floor(harmonic_curtime_left_[i])] >> i;
		}
		sigma = sigma / 2 + 128;
		return floor(volume_ * sigma / 100);
	}

	uint8_t GetRight() {
		int sigma = 0;
		for(int i = 0; i < harmonics_; i++) {
			sigma += waveTable_[(int)floor(harmonic_curtime_right_[i])] >> i;
		}
		sigma = sigma / 2 + 128;
		return floor(volume_ * sigma / 100);
	}
};

int fillBuffer(const void *input, void *output, unsigned long frameCount, const PaStreamCallbackTimeInfo *timeinfo, PaStreamCallbackFlags statusFlags, void *userData) {
	CurrentState *cur_state = (CurrentState*)state;
	sampleType *out = (sampleType*)output;
	for(unsigned long i = 0; i < frameCount; i++) {
		cur_state->IncerementCurtimes();
		out[0] = cur_state->GetLeft();
		out[1] = cur_state->GetRight();
		out += 2 * sizeof(sampleType);
	}
}

SoundManager::SoundManager(QObject *parent) :
	QObject(parent),
	stream_(NULL) {
	PaError err = Pa_Initialize();
	if (err != paNoError) {
		qDebug() << "Failed to initialize portaudio: " << Pa_GetErrorText(err);
	}
}

SoundManager::~SoundManager() {
	Pa_CloseStream(stream_);
	Pa_Terminate();
}

void SoundManager::initOut() {
	PaStream *stream;
	PaError err = Pa_OpenDefaultStream(&stream, 0, 2, paUInt8, kSampleRate, 0, fillBuffer, NULL);
	if (err != paNoError) {
		qDebug() << "Failed to open a stream: " << Pa_GetErrorText(err);
		return;
	}
	CurrentState *old_state = state.fetchAndStoreOrdered(new CurrentState(kSampleRate));
	if (old_state != NULL) { delete old_state; }
	stream_ = stream;
}

void SoundManager::start() {
	if (stream_ == NULL) {
		initOut();
	}
	if (stream_ == NULL) { return; }
	PaError err = Pa_StartStream(stream_);
	if (err != paNoError) {
		qDebug() << "Failed to start stream: " << Pa_GetErrorText(err);
	}
	qDebug() << "start";
}

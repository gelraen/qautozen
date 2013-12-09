#include "sound_thread.h"
#include <QAtomicPointer>
#include <QDebug>
#include <vector>
#include <cmath>

using std::floor;
using std::pow;
using std::sin;
using std::vector;

const int kSampleRate = 8000;
const int kMaxHarmonics = 10;
const int kDefaultHarmonics = 3;
const int kBeatMax = 40;
const int kSampleSize = 8;
const int kChannelCount = 2; // stereo
const int kBufferLenMs = 125;

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
	CurrentState(int sampleRate) : volume_(100), base_(300), beat_(25), sample_rate_(sampleRate) {
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

SoundManager::SoundManager(QObject *parent) :
	QObject(parent),
	out_(NULL),
	state_(new CurrentState(kSampleRate)),
	out_device_(NULL) {
}

SoundManager::~SoundManager() {
	delete state_;
}

void SoundManager::initOut() {
	QAudioFormat format;
	format.setChannelCount(kChannelCount);
	format.setSampleRate(kSampleRate);
	format.setSampleSize(kSampleSize);
	format.setSampleType(QAudioFormat::SignedInt);
	format.setCodec("audio/pcm");

	QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
	if (!info.isFormatSupported(format)) {
		qDebug() << "Desired audio format is not supported!";
		return;
	}

	out_ = new QAudioOutput(format, this);
	out_->setBufferSize(kSampleRate * kBufferLenMs / 1000 * kSampleSize / 8 * kChannelCount);
	out_->setNotifyInterval(kBufferLenMs / 2);

	connect(out_, SIGNAL(stateChanged(QAudio::State)), SLOT(onOutStateChanged(QAudio::State)));
	connect(out_, SIGNAL(notify()), SLOT(onOutNotify()));
}

void SoundManager::start() {
	if (out_ == NULL) {
		initOut();
	}
	if (out_ == NULL) { return; }
	if (out_device_ == NULL) {
		out_device_ = out_->start();
	}
	if (out_device_ == NULL) { return; }
	//out_->resume();

	//qDebug() << "State: " << out_->state() << ", Error: " << out_->error() << ", FreeBytes: " << out_->bytesFree();
	fillBuffer(state_, out_device_, out_->bytesFree());
	qDebug() << "start";
	//qDebug() << "State: " << out_->state() << ", Error: " << out_->error() << ", FreeBytes: " << out_->bytesFree();
	//out_->resume();
	//qDebug() << "State: " << out_->state() << ", Error: " << out_->error() << ", FreeBytes: " << out_->bytesFree();
}

void SoundManager::onOutStateChanged(QAudio::State state) {
	qDebug() << "State changed";
	qDebug() << "State: " << out_->state() << ", Error: " << out_->error() << ", FreeBytes: " << out_->bytesFree();
	if (out_ == NULL || out_device_ == NULL) { return; }
	fillBuffer(state_, out_device_, out_->bytesFree());
}

void SoundManager::onOutNotify() {
	qDebug() << "Notify";
	qDebug() << "State: " << out_->state() << ", Error: " << out_->error() << ", FreeBytes: " << out_->bytesFree();
	if (out_ == NULL || out_device_ == NULL) { return; }
	fillBuffer(state_, out_device_, out_->bytesFree());
}

void SoundManager::fillBuffer(CurrentState *state, QIODevice *out, int freeBytes) {
	int bytesWritten = 0, n;
	for(int i = 0; i < freeBytes / (2 * kSampleSize / 8); i++) {
		state->IncerementCurtimes();
		uint8_t tmp = state->GetLeft();
		n = out->write((char*)&tmp, sizeof(tmp));
		qDebug() << "Write returned " << n;
		if (n < 0) {
			qDebug() << "Write failed: " << out->errorString();
		} else {
			bytesWritten += n;
		}
		tmp = state->GetRight();
		n = out->write((char*)&tmp, sizeof(tmp));
		if (n < 0) {
			qDebug() << "Write failed: " << out->errorString();
		} else {
			bytesWritten += n;
		}
	}
	qDebug() << "Written " << bytesWritten << " bytes";
}

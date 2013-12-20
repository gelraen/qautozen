#include "file_processor.h"
#include <QApplication>
#include <QDebug>
#include <QIODevice>
#include <QRegExp>
#include <QTimer>

enum UpdateType { VOLUME, BEAT };
struct CommandState {
  int updates_left;
  int update_magnitude;
  int update_interval;
  int update_type;
};

FileProcessor::FileProcessor() : sound_manager_(NULL),
    file_(NULL), d_(new CommandState()) {}

FileProcessor::~FileProcessor() {}

void FileProcessor::setSoundManager(SoundManager* p) {
  sound_manager_ = p;
}

void FileProcessor::playFile(QString filename) {
  file_ = new QFile(filename, this);
  if (!file_->open(QIODevice::ReadOnly)) {
    qDebug() << "Failed to open file " << filename << " : " << file_->errorString();
    delete file_;
    file_ = NULL;
    return;
  }

  emit sequenceStarted();
  QTimer::singleShot(0, this, SLOT(readNextCommand()));
}

void FileProcessor::stopSequence() {
  if (file_ != NULL) {
    file_->close();
    delete file_;
    file_ = NULL;
  }
  emit sequenceStopped();
}

void FileProcessor::readNextCommand() {
  if (file_ == NULL) { return; }
  if (file_->atEnd()) {
    stopSequence();
    return;
  }

  QString line = file_->readLine();

  QStringList tokens = line.split(QRegExp("[\\s,]"), QString::SkipEmptyParts);
  qDebug() << "Command: " << tokens;

  if (tokens.at(0) == "BASE" && tokens.length() == 2) {
    commandBase(tokens);
  } else if (tokens.at(0) == "VOLUME" && tokens.length() == 2) {
    commandVolume(tokens);
  } else if (tokens.at(0) == "SET" && tokens.length() == 2) {
    commandSet(tokens);
  } else if (tokens.at(0) == "HARMONICS" && tokens.length() == 2) {
    commandHarmonics(tokens);
  } else if (tokens.at(0) == "HOLD" && tokens.length() == 2) {
    commandHold(tokens);
  } else if (tokens.at(0) == "PAUSE" && tokens.length() == 1) {
    commandPause(tokens);
  } else if (tokens.at(0) == "END" && tokens.length() == 1) {
    commandEnd(tokens);
  } else if (tokens.at(0) == "EXIT" && tokens.length() == 1) {
    commandExit(tokens);
  } else if (tokens.at(0) == "FADE" && tokens.length() == 3) {
    commandFade(tokens);
  } else if (tokens.at(0) == "SLIDE" && tokens.length() == 3) {
    commandSlide(tokens);
  } else {
    qDebug() << "Invalid command: " << tokens;
    stopSequence();
  }
}

void FileProcessor::commandBase(const QStringList& cmd) {
  bool ok;
  int v = cmd.at(1).toInt(&ok);
  if (!ok) {
    qDebug() << "Invalid BASE command: " << cmd;
    stopSequence();
    return;
  }
  if (sound_manager_ != NULL) {
    sound_manager_->setBase(v);
  }
  QTimer::singleShot(0, this, SLOT(readNextCommand()));
}

void FileProcessor::commandVolume(const QStringList& cmd) {
  bool ok;
  int v = cmd.at(1).toInt(&ok);
  if (!ok) {
    qDebug() << "Invalid VOLUME command: " << cmd;
    stopSequence();
    return;
  }
  if (sound_manager_ != NULL) {
    sound_manager_->setVolume(v);
  }
  QTimer::singleShot(0, this, SLOT(readNextCommand()));
}

void FileProcessor::commandSet(const QStringList& cmd) {
  bool ok;
  int v = cmd.at(1).toInt(&ok);
  if (!ok) {
    qDebug() << "Invalid SET command: " << cmd;
    stopSequence();
    return;
  }
  if (sound_manager_ != NULL) {
    sound_manager_->setBeat(v);
  }
  QTimer::singleShot(0, this, SLOT(readNextCommand()));
}

void FileProcessor::commandHarmonics(const QStringList& cmd) {
  bool ok;
  int v = cmd.at(1).toInt(&ok);
  if (!ok) {
    qDebug() << "Invalid HARMONICS command: " << cmd;
    stopSequence();
    return;
  }
  if (sound_manager_ != NULL) {
    sound_manager_->setHarmonics(v);
  }
  QTimer::singleShot(0, this, SLOT(readNextCommand()));
}

void FileProcessor::commandHold(const QStringList& cmd) {
  bool ok;
  int v = cmd.at(1).toInt(&ok);
  if (!ok) {
    qDebug() << "Invalid HOLD command: " << cmd;
    stopSequence();
    return;
  }
  QTimer::singleShot(v * 1000, this, SLOT(readNextCommand()));
}

void FileProcessor::commandPause(const QStringList& /* cmd */) {
  if (sound_manager_ != NULL) {
    sound_manager_->stop();
  }
  emit sequencePaused();
}

void FileProcessor::continueSequence() {
  if (sound_manager_ != NULL) {
    sound_manager_->start();
  }
  QTimer::singleShot(0, this, SLOT(readNextCommand()));
}

void FileProcessor::commandEnd(const QStringList& /* cmd */) {
  stopSequence();
}

void FileProcessor::commandExit(const QStringList& /* cmd */) {
  qApp->quit();
}

void FileProcessor::commandFade(const QStringList& cmd) {
  int cur = -1;
  bool ok1, ok2;
  int target = cmd.at(1).toInt(&ok1);
  int interval = cmd.at(2).toInt(&ok2);
  if (!ok1 || !ok2) {
    qDebug() << "Invalid FADE command: " << cmd;
    stopSequence();
    return;
  }
  if (sound_manager_ != NULL) {
    cur = sound_manager_->getVolume();
    d_->update_magnitude = target > cur ? 1 : -1;
    d_->updates_left = abs(target - cur);
    d_->update_interval = interval * 1000 / abs(target - cur);
    d_->update_type = VOLUME;
    QTimer::singleShot(d_->update_interval, this, SLOT(doAdjustment()));
  }
}

void FileProcessor::commandSlide(const QStringList& cmd) {
  int cur = -1;
  bool ok1, ok2;
  int target = cmd.at(1).toInt(&ok1);
  int interval = cmd.at(2).toInt(&ok2);
  if (!ok1 || !ok2) {
    qDebug() << "Invalid SLIDE command: " << cmd;
    stopSequence();
    return;
  }
  if (sound_manager_ != NULL) {
    cur = sound_manager_->getBeat();
    d_->update_magnitude = target > cur ? 1 : -1;
    d_->updates_left = abs(target - cur);
    d_->update_interval = interval * 1000 / abs(target - cur);
    d_->update_type = VOLUME;
    QTimer::singleShot(d_->update_interval, this, SLOT(doAdjustment()));
  }
}

void FileProcessor::doAdjustment() {
  if (d_->updates_left > 0) {
    d_->updates_left--;
    if (sound_manager_ != NULL) {
      switch (d_->update_type) {
      case VOLUME:
        sound_manager_->setVolume(sound_manager_->getVolume() + d_->update_magnitude);
        break;
      case BEAT:
        sound_manager_->setBeat(sound_manager_->getBeat() + d_->update_magnitude);
        break;
      }
    }
    QTimer::singleShot(d_->update_interval, this, SLOT(doAdjustment()));
  } else {
    QTimer::singleShot(0, this, SLOT(readNextCommand()));
  }
}

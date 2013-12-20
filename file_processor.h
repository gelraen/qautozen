#pragma once

#include <QFile>
#include <QObject>
#include <QStringList>

#include "sound_thread_portaudio.h"

struct CommandState;

class FileProcessor : public QObject {
  Q_OBJECT
 public:
  FileProcessor();
  virtual ~FileProcessor();
  void setSoundManager(SoundManager*);
 public slots:
  void playFile(QString);
  void stopSequence();
  void continueSequence();
 signals:
  void sequenceStarted();
  void sequenceStopped();
  void sequencePaused();
  void sequenceResumed();
 private slots:
  void readNextCommand();
  void doAdjustment();
 private:
  void commandBase(const QStringList&);
  void commandVolume(const QStringList&);
  void commandSet(const QStringList&);
  void commandHarmonics(const QStringList&);
  void commandHold(const QStringList&);
  void commandPause(const QStringList&);
  void commandEnd(const QStringList&);
  void commandExit(const QStringList&);
  void commandFade(const QStringList&);
  void commandSlide(const QStringList&);

 private:
  SoundManager* sound_manager_;
  QFile* file_;
  CommandState* d_;
  bool paused_;
};

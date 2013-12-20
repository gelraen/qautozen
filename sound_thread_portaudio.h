#pragma once

#include <QObject>
#include <portaudio.h>

struct CurrentState;

const int kBaseMin = 50;
const int kBaseMax = 1000;
const int kBaseDefault = 300;
const int kBeatMin = 0;
const int kBeatMax = 40;
const int kBeatDefault = 20;
const int kHarmonicsMin = 1;
const int kHarmonicsMax = 10;
const int kHarmonicsDefault = 3;
const int kVolumeMin = 0;
const int kVolumeMax = 100;
const int kVolumeDefault = 50;

class SoundManager : public QObject {
  Q_OBJECT
 public:
  SoundManager(QObject* parent = NULL);
  virtual ~SoundManager();
 public slots:
  void start();
  void stop();
  void setBeat(int);
  void setBase(int);
  void setVolume(int);
  void setHarmonics(int);
signals:
  void beatChanged(int);
  void baseChanged(int);
  void volumeChanged(int);
  void harmonicsChanged(int);

 public:
  int getBeat();
  int getBase();
  int getVolume();
  int getHarmonics();

 private:
  void initOut();
  PaStream* stream_;
};

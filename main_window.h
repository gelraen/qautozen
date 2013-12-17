#pragma once

#include <QMainWindow>
#include <QPushButton>
#include <QSlider>

#include "sound_thread_portaudio.h"

class AutozenWindow : public QMainWindow {
  Q_OBJECT
 public:
  AutozenWindow();
  virtual ~AutozenWindow();
 private slots:
  void beatChanged(int);
  void baseChanged(int);
  void volumeChanged(int);

 private:
  QLayout* createMainLayout();
  QLayout* createSlidersLayout();
  QLayout* createButtonsLayout();
  QLayout* createBeatSlider();
  QLayout* createBaseSlider();
  QLayout* createVolumeSlider();

 private:
  QPushButton* start_button_;
  QPushButton* stop_button_;
  SoundManager* sound_manager_;
  QSlider* beat_slider_;
  QSlider* base_slider_;
  QSlider* volume_slider_;
};

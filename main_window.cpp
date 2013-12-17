#include "main_window.h"
#include <QHBoxLayout>
#include <QLabel>
#include <Qt>
#include <QVBoxLayout>

QLayout* AutozenWindow::createMainLayout() {
  QVBoxLayout* l = new QVBoxLayout();
  l->addLayout(createSlidersLayout());
  l->addLayout(createButtonsLayout());
  return l;
}

QLayout* AutozenWindow::createSlidersLayout() {
  QHBoxLayout* l = new QHBoxLayout();
  l->addLayout(createBeatSlider());
  l->addLayout(createBaseSlider());
  l->addLayout(createVolumeSlider());
  return l;
}

QLayout* AutozenWindow::createBeatSlider() {
  QVBoxLayout* l = new QVBoxLayout();
  QLabel* beat_label = new QLabel();
  l->addWidget(beat_label);
  beat_label->setNum(kBeatDefault);

  beat_slider_ = new QSlider(Qt::Vertical);
  beat_slider_->setMinimum(kBeatMin);
  beat_slider_->setMaximum(kBeatMax);
  beat_slider_->setValue(kBeatDefault);
  l->addWidget(beat_slider_);

  QLabel* t = new QLabel(tr("Beat"));
  l->addWidget(t);

  connect(beat_slider_, SIGNAL(valueChanged(int)), beat_label,
          SLOT(setNum(int)));
  connect(beat_slider_, SIGNAL(valueChanged(int)), sound_manager_,
          SLOT(setBeat(int)));
  return l;
}

QLayout* AutozenWindow::createBaseSlider() {
  QVBoxLayout* l = new QVBoxLayout();
  QLabel* base_label = new QLabel();
  l->addWidget(base_label);
  base_label->setNum(kBaseDefault);

  base_slider_ = new QSlider(Qt::Vertical);
  base_slider_->setMinimum(kBaseMin);
  base_slider_->setMaximum(kBaseMax);
  base_slider_->setValue(kBaseDefault);
  l->addWidget(base_slider_);

  QLabel* t = new QLabel(tr("Base"));
  l->addWidget(t);

  connect(base_slider_, SIGNAL(valueChanged(int)), base_label,
          SLOT(setNum(int)));
  connect(base_slider_, SIGNAL(valueChanged(int)), sound_manager_,
          SLOT(setBase(int)));
  return l;
}

QLayout* AutozenWindow::createVolumeSlider() {
  QVBoxLayout* l = new QVBoxLayout();
  QLabel* volume_label = new QLabel();
  l->addWidget(volume_label);
  volume_label->setNum(kVolumeDefault);

  volume_slider_ = new QSlider(Qt::Vertical);
  volume_slider_->setMinimum(kVolumeMin);
  volume_slider_->setMaximum(kVolumeMax);
  volume_slider_->setValue(kVolumeDefault);
  l->addWidget(volume_slider_);

  QLabel* t = new QLabel(tr("Volume"));
  l->addWidget(t);

  connect(volume_slider_, SIGNAL(valueChanged(int)), volume_label,
          SLOT(setNum(int)));
  connect(volume_slider_, SIGNAL(valueChanged(int)), sound_manager_,
          SLOT(setVolume(int)));
  return l;
}

QLayout* AutozenWindow::createButtonsLayout() {
  QHBoxLayout* l = new QHBoxLayout();
  start_button_ = new QPushButton(tr("Start"), this);
  stop_button_ = new QPushButton(tr("Stop"), this);
  l->addWidget(start_button_);
  l->addWidget(stop_button_);
  return l;
}

AutozenWindow::AutozenWindow() : QMainWindow(NULL) {
  sound_manager_ = new SoundManager(NULL);

  QWidget* w = new QWidget();
  w->setLayout(createMainLayout());
  setCentralWidget(w);
  connect(start_button_, SIGNAL(clicked(bool)), sound_manager_, SLOT(start()));
  connect(stop_button_, SIGNAL(clicked(bool)), sound_manager_, SLOT(stop()));
}

AutozenWindow::~AutozenWindow() {}

void AutozenWindow::beatChanged(int v) {
  if (v != beat_slider_->value()) {
    beat_slider_->setValue(v);
  }
}

void AutozenWindow::baseChanged(int v) {
  if (v != base_slider_->value()) {
    base_slider_->setValue(v);
  }
}

void AutozenWindow::volumeChanged(int v) {
  if (v != volume_slider_->value()) {
    volume_slider_->setValue(v);
  }
}

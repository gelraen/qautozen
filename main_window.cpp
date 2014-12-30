#include "main_window.h"

#include <QFileDialog>
#include <QGridLayout>
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
  l->addStretch();
  l->addLayout(createBeatSlider());
  l->addStretch();
  l->addLayout(createBaseSlider());
  l->addStretch();
  l->addLayout(createVolumeSlider());
  l->addStretch();
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
  beat_slider_->setFocusPolicy(Qt::StrongFocus);
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
  base_slider_->setFocusPolicy(Qt::StrongFocus);
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
  volume_slider_->setFocusPolicy(Qt::StrongFocus);
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
  QGridLayout* l = new QGridLayout();
  start_button_ = new QPushButton(tr("Start"), this);
  stop_button_ = new QPushButton(tr("Stop"), this);
  open_sequence_ = new QPushButton(tr("Open file"), this);
  resume_sequence_ = new QPushButton(tr("Resume sequence"), this);
  resume_sequence_->setEnabled(false);
  l->addWidget(start_button_, 0, 0);
  l->addWidget(stop_button_, 0, 1);
  l->addWidget(open_sequence_, 1, 0);
  l->addWidget(resume_sequence_, 1, 1);

  connect(open_sequence_, SIGNAL(clicked()), this, SLOT(openFile()));
  return l;
}

AutozenWindow::AutozenWindow() : QMainWindow(NULL) {
  sound_manager_ = new SoundManager(NULL);

  file_processor_.setSoundManager(sound_manager_);

  setWindowFlags(windowFlags() | Qt::WindowFullscreenButtonHint);
  QWidget* w = new QWidget();
  w->setLayout(createMainLayout());
  setCentralWidget(w);
  connect(start_button_, SIGNAL(clicked(bool)), sound_manager_, SLOT(start()));
  connect(stop_button_, SIGNAL(clicked(bool)), sound_manager_, SLOT(stop()));
  connect(sound_manager_, SIGNAL(beatChanged(int)), this, SLOT(beatChanged(int)));
  connect(sound_manager_, SIGNAL(baseChanged(int)), this, SLOT(baseChanged(int)));
  connect(sound_manager_, SIGNAL(volumeChanged(int)), this, SLOT(volumeChanged(int)));
  connect(&file_processor_, SIGNAL(sequenceStarted()), this, SLOT(sequenceStarted()));
  connect(&file_processor_, SIGNAL(sequenceStopped()), this, SLOT(sequenceStopped()));
  connect(&file_processor_, SIGNAL(sequencePaused()), this, SLOT(sequencePaused()));
  connect(&file_processor_, SIGNAL(sequenceResumed()), this, SLOT(sequenceResumed()));
  connect(resume_sequence_, SIGNAL(clicked()), &file_processor_, SLOT(continueSequence()));
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

void AutozenWindow::sequenceStarted() {
  beat_slider_->setEnabled(false);
  base_slider_->setEnabled(false);
  volume_slider_->setEnabled(false);
  open_sequence_->setEnabled(false);
  start_button_->setEnabled(false);
  stop_button_->setEnabled(false);
}

void AutozenWindow::sequenceStopped() {
  beat_slider_->setEnabled(true);
  base_slider_->setEnabled(true);
  volume_slider_->setEnabled(true);
  open_sequence_->setEnabled(true);
  start_button_->setEnabled(true);
  stop_button_->setEnabled(true);
}

void AutozenWindow::sequencePaused() {
  resume_sequence_->setEnabled(true);
}

void AutozenWindow::sequenceResumed() {
  resume_sequence_->setEnabled(false);
}

void AutozenWindow::openFile() {
  QString filename = QFileDialog::getOpenFileName(this,
    tr("Open sequence file"), QString(), tr("Sequence files (*.seq);;All files (*.*)"));
  if (!filename.isNull()) {
    file_processor_.playFile(filename);
  }
}


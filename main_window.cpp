#include "main_window.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

AutozenWindow::AutozenWindow() : QMainWindow(NULL) {
	sound_manager_ = new SoundManager(NULL);
	sound_manager_->moveToThread(&sound_thread_);

	QWidget *w = new QWidget();
	QVBoxLayout *mainLayout = new QVBoxLayout();
	w->setLayout(mainLayout);
	QHBoxLayout *buttons = new QHBoxLayout();
	mainLayout->addLayout(buttons);
	start_button_ = new QPushButton(tr("Start"), this);
	stop_button_ = new QPushButton(tr("Stop"), this);
	buttons->addWidget(start_button_);
	buttons->addWidget(stop_button_);
	setCentralWidget(w);
	connect(start_button_, SIGNAL(clicked(bool)), sound_manager_, SLOT(start()));
	connect(stop_button_, SIGNAL(clicked(bool)), sound_manager_, SLOT(stop()));
	sound_thread_.start();
}

AutozenWindow::~AutozenWindow() {

}

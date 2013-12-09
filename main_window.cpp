#include "main_window.h"

AutozenWindow::AutozenWindow() : QMainWindow(NULL) {
	sound_manager_ = new SoundManager(NULL);
	sound_manager_->moveToThread(&sound_thread_);
	start_button_ = new QPushButton(tr("Start"), this);
	setCentralWidget(start_button_);
	connect(start_button_, SIGNAL(clicked(bool)), sound_manager_, SLOT(start()));
	sound_thread_.start();
}

AutozenWindow::~AutozenWindow() {

}

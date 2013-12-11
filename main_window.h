#include <QMainWindow>
#include <QPushButton>
#include <QThread>

#include "sound_thread_portaudio.h"

class AutozenWindow : public QMainWindow {
	Q_OBJECT
public:
	AutozenWindow();
	virtual ~AutozenWindow();
private:
	QPushButton *start_button_;
	QPushButton *stop_button_;
	QThread sound_thread_;
	SoundManager *sound_manager_;
};

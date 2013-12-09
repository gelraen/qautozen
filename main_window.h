#include <QMainWindow>
#include <QPushButton>
#include <QThread>

#include "sound_thread.h"

class AutozenWindow : public QMainWindow {
	Q_OBJECT
public:
	AutozenWindow();
	virtual ~AutozenWindow();
private:
	QPushButton *start_button_;
	QThread sound_thread_;
	SoundManager *sound_manager_;
};

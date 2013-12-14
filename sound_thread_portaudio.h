#include <QObject>
#include <portaudio.h>

struct CurrentState;

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
signals:
	void beatChanged(int);
	void baseChanged(int);
	void volumeChanged(int);
public:
	int getBeat();
	int getBase();
	int getVolume();
private:
	void initOut();
	PaStream *stream_;
};

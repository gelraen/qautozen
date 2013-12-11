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
private:
	void initOut();
	PaStream *stream_;
};

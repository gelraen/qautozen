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
private:
	void initOut();
	PaStream *stream_;
};

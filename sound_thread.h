#include <QAudio>
#include <QAudioOutput>
#include <QIODevice>

struct CurrentState;

class SoundManager : public QObject {
	Q_OBJECT
public:
	SoundManager(QObject* parent = NULL);
	virtual ~SoundManager();
public slots:
	void start();
private slots:
	void onOutStateChanged(QAudio::State);
	void onOutNotify();
private:
	void initOut();
	void fillBuffer(CurrentState*, QIODevice*, int);
	QAudioOutput *out_;
	QIODevice *out_device_;
	CurrentState *state_;
};

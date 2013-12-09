#include <QApplication>
#include "main_window.h"

int main(int argc, char *argv[]) {
	QApplication app(argc, argv);
	AutozenWindow w;
	w.show();
	return app.exec();
}

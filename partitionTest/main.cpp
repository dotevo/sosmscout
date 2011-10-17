#include <QtGui/QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;
    w.show();
	//Dla windowsa CE fullscreen
    #ifdef Q_OS_WINCE_STD
        w.showFullScreen();
    #endif

    return a.exec();
}

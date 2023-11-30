#include <QApplication>
#include "SimpleAudioApplicationWindow.h"

int main(int argc, char **argv) {
    QApplication a(argc, argv);
    SimpleAudioApplicationWindow win;
    win.show();
    a.exec();
}

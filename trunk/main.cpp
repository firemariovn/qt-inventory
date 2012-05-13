#include <QtGui>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setProperty("user_id", 0);
    a.setProperty("ini", QDir::home().filePath(".inventory.ini"));

    MainWindow w;
    w.show();

    return a.exec();
}

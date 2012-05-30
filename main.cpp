#include <QtGui>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setProperty("user_id", 0);
    a.setProperty("ini", QDir::home().filePath(".inventory.ini"));

    QTranslator updmgr_translator;
    QString locale = QLocale::system().name();
    if(updmgr_translator.load(QString(":/languages/UpdateManager_") + locale)) a.installTranslator(&updmgr_translator);

    MainWindow w;
    w.show();

    return a.exec();
}

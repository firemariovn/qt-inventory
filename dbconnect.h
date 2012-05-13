#ifndef DBCONNECT_H
#define DBCONNECT_H

#include "ui_dbconnect.h"
#include <QDialog>

class QSqlDatabase;
class Logger;

namespace Ui {
    class DbConnectDialog;
}
class DbConnectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DbConnectDialog(QSqlDatabase* db ,QWidget * parent = 0, Qt::WindowFlags f = 0);
    ~DbConnectDialog();

    inline void setLogger(Logger* l){ logger = l; }

public slots:
    void setDbHost(const QString& host);
    void setDbName(const QString& name);
    void setDbUser(const QString& user);
    void setDbPass(const QString& pass);
    void accept();

private slots:
    void setDatabaseTypes(const QString& type);
    void chooseSqliteDatabase();
    void showError(const QString& msg, const int secs=3);
    void hideError();
    void newDatabase();

protected:
    void keyPressEvent(QKeyEvent* e);

private:
    void loadFromSettings();
    void saveToSettings();

    Ui::DbConnectDialog* ui;
    QSqlDatabase* _db;
    Logger* logger;

};

#endif // DBCONNECT_H

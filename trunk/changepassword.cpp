#include "changepassword.h"
#include "ui_changepassword.h"

#include <QtSql>
#include <QtGui>

ChangePassword::ChangePassword(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChangePassword)
{
    password = "";
    id = 0;

    ui->setupUi(this);
    ui->error_label->hide();
}

ChangePassword::~ChangePassword()
{
    delete ui;
}

void ChangePassword::hideError()
{
    ui->error_label->hide();
}

void ChangePassword::showError(const QString &msg, const int secs)
{
    ui->error_label->setText(msg);
    ui->error_label->show();

    QTimer::singleShot(secs*1000, this, SLOT(hideError()));
}

void ChangePassword::loadUser()
{
    loadUser(qApp->property("user_id").toInt());
}

void ChangePassword::loadUser(const int user_id)
{
    id = user_id;

    ui->old_password_lineEdit->clear();
    ui->new_password_lineEdit->clear();
    ui->confirmation_lineEdit->clear();

    QSqlQuery* query = new QSqlQuery();
    if(!query->exec(QString("SELECT password FROM users WHERE `id`=%1")
                    .arg(id)
                    )){
        showError(query->lastError().text(), 5);
       // qDebug() << "Last executed query: " << query->lastQuery();
        return;
    }
    if(!query->first()){
        showError(tr("Unknown user info!"));
        return;
    }
    password = query->value(0).toString();
}

bool ChangePassword::saveUserPassword()
{
    if(!id) return false;
    QString old_password = ui->old_password_lineEdit->text();
    QString new_password = ui->new_password_lineEdit->text();
    QString confirmation = ui->confirmation_lineEdit->text();
    if(old_password != password){
        showError(tr("Incorect old password.\nPlease check for turned on or off capslock/numlock!"));
        return false;
    }
    if(new_password.isEmpty()){
        showError(tr("Empty password not allowed"));
        return false;
    }
    if(new_password != confirmation){
        showError(tr("Passwords do not match!"));
        return false;
    }
    if(new_password == old_password){
        showError(tr("No changes were registered.\nCheck the new password again!"));
        return false;
    }
    QSqlQuery* query = new QSqlQuery();
    if(!query->exec(QString("UPDATE users SET `password`='%1' WHERE `id`=%2")
                    .arg(new_password)
                    .arg(id)
                    )){
        showError(query->lastError().text(), 5);
       // qDebug() << "Last executed query: " << query->lastQuery();
        return false;
    }
    return true;
}

void ChangePassword::accept()
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    if(!saveUserPassword()){
        QApplication::restoreOverrideCursor();
        return;
    }
    QApplication::restoreOverrideCursor();
    QDialog::accept();
}

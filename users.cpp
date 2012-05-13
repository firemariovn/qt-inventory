#include "users.h"
#include "ui_users.h"
#include "logger.h"

#include <QtSql>
#include <QtGui>

Users::Users(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Users)
{
    ui->setupUi(this);
    ui->error_label->hide();
    setId(0);
    setEditMode(false);

    logger = 0;
}

Users::~Users()
{
    delete ui;
}

void Users::accept()
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    if(!saveUser()){
        QApplication::restoreOverrideCursor();
        return;
    }
    QApplication::restoreOverrideCursor();
    QDialog::accept();
}

void Users::showError(const QString &msg, const int secs)
{
    ui->error_label->setText(msg);
    ui->error_label->show();
    QTimer::singleShot(secs*1000, ui->error_label, SLOT(hide()));
}

bool Users::checkPassword()
{
    if(ui->password_lineEdit->text() != ui->confirm_lineEdit->text()){
        showError(tr("Passwords do not match!"));
        return false;
    }
    return true;
}

void Users::loadUser(int user_id){
    setId(user_id);
    if(!getId()) return;
    QSqlQuery* query = new QSqlQuery(QSqlDatabase::database());
    if(!query->exec(QString("SELECT * FROM `users` WHERE `id`=%1")
                        .arg(getId()))){
        qDebug() << "Can not get user info from database. Sql error: " << query->lastError().text();
        return;
    }
    if(!query->first()){
        qDebug() << "Sql Error: " << query->lastError();
        qDebug() << "Last executed query: " << query->lastQuery();
        return;
    }
    QSqlRecord record = query->record();
    ui->username_lineEdit->setText(record.value("username").toString());
    ui->firstname_lineEdit->setText(record.value("firstname").toString());
    ui->lastname_lineEdit->setText(record.value("lastname").toString());
    ui->password_lineEdit->setText(record.value("password").toString());
    ui->confirm_lineEdit->setText(record.value("password").toString());
    ui->email_lineEdit->setText(record.value("email").toString());
}

bool Users::saveUser()
{
    if(ui->username_lineEdit->text().isEmpty()){
        showError(tr("Empty username not allowed!"));
        return false;
    }
    if(ui->password_lineEdit->text().isEmpty()){
        showError(tr("Empty password not allowed!"));
        return false;
    }
    QSqlQuery* query = new QSqlQuery(QSqlDatabase::database());
    if(!getEditMode()){
        if(!query->exec(QString("SELECT * FROM `users` WHERE `username` LIKE '%1' AND `deleted`=0").arg(ui->username_lineEdit->text()))){
            if(logger) logger->writeLog(Logger::Error, Logger::Users,
                                        tr("Can not connect with database. Sql error:\n%1").arg(query->lastError().text())
                                        );
            return false;
        }
        if(query->first()){
            showError(tr("There is another user with the supplied name.\nThe operation is interrupted!"), 5);
            return false;
        }
        if(!this->checkPassword()) return false;
        if(!query->exec(QString("INSERT INTO users (`username`,`firstname`,`lastname`,`password`,`email`,`deleted`) VALUES ('%1', '%2', '%3', '%4', '%5', 0)")
                        .arg(ui->username_lineEdit->text())
                        .arg(ui->firstname_lineEdit->text())
                        .arg(ui->lastname_lineEdit->text())
                        .arg(ui->password_lineEdit->text())
                        .arg(ui->email_lineEdit->text())
                        )){
            if(logger) logger->writeLog(Logger::Error, Logger::Users,
                                        tr("Can not save user into database. Sql error:\n%1")
                                        .arg(query->lastError().text())
                                        );
            return false;
        }
        int user_id = query->lastInsertId().toInt();
        if(logger) logger->writeLog(Logger::Add, Logger::Users,
                                    tr("New user was defined.\nUser: %1\nFirst name: %2\nLast name: %3")
                                    .arg(ui->username_lineEdit->text())
                                    .arg(ui->firstname_lineEdit->text())
                                    .arg(ui->lastname_lineEdit->text())
                                    );
        if(!query->exec(QString("INSERT INTO users_rights (`user_id`) VALUES ('%1')")
                        .arg(user_id)
                        )){
            if(logger) logger->writeLog(Logger::Error, Logger::Users,
                                        tr("Can not save user rights into database. Sql error:\n%1")
                                        .arg(query->lastError().text())
                                        );
            return false;
        }
    }
    else {
        if(!this->checkPassword()) return false;
        if(!query->exec(QString("UPDATE users SET "
                        "`username`='%1',`firstname`='%2',`lastname`='%3',`password`='%4',`email`='%5' "
                        "WHERE `id`='%6'")
                        .arg(ui->username_lineEdit->text())
                        .arg(ui->firstname_lineEdit->text())
                        .arg(ui->lastname_lineEdit->text())
                        .arg(ui->password_lineEdit->text())
                        .arg(ui->email_lineEdit->text())
                        .arg(getId())
                        )){
            if(logger) logger->writeLog(Logger::Error, Logger::Users,
                                        tr("Sql error:\n%1").arg(query->lastError().text())
                                        );
            return false;
        }
        else
            if(logger) logger->writeLog(Logger::Edit, Logger::Users,
                                        tr("%1 user data has been changed.").arg(ui->username_lineEdit->text())
                                        );
    }
    return true;
}

void Users::setEditMode(const bool edit)
{
    _edit = edit;
    ui->username_lineEdit->setDisabled(edit);
    this->setWindowTitle(tr("Users (%1 user)").arg(edit ? tr("edit") : tr("new")));
}


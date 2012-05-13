#include "dbconnect.h"
#include "logger.h"

#include <QtSql>
#include <QtGui>
//#include <QFileDialog>
//#include <QSettings>

DbConnectDialog::DbConnectDialog(QSqlDatabase* db, QWidget *parent, Qt::WindowFlags f) :
    QDialog(parent, f), ui(new Ui::DbConnectDialog)
{
    ui->setupUi(this);
    ui->error_label->hide();
    _db = db;
    logger = 0;

    QMenu* database = new QMenu(ui->open_toolButton);

    QAction* new_database = new QAction(QIcon(":/Icons/icons/Database.png"), tr("New database"), database);
    database->addAction(new_database);
    ui->open_toolButton->setMenu(database);

    //connect(ui->dbtype_comboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(setDatabaseTypes(QString)));
    connect(ui->database_toolButton, SIGNAL(clicked()), this, SLOT(chooseSqliteDatabase()));
    connect(new_database, SIGNAL(triggered()), this, SLOT(newDatabase()));

    //ui->dbtype_comboBox->addItems(db->drivers());
    this->setDatabaseTypes("QSQLITE");
    this->loadFromSettings();
}

DbConnectDialog::~DbConnectDialog()
{
    delete ui;
}

void DbConnectDialog::setDbHost(const QString &host)
{
    ui->host_lineEdit->setText(host);
}

void DbConnectDialog::setDbName(const QString &name)
{
    ui->database_lineEdit->setText(name);
}

void DbConnectDialog::setDbUser(const QString &user)
{
    ui->user_lineEdit->setText(user);
}

void DbConnectDialog::setDbPass(const QString &pass)
{
    ui->password_lineEdit->setText(pass);
}

void DbConnectDialog::accept()
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    //QString type = ui->dbtype_comboBox->currentText();
    QString type = "QSQLITE";
    if(!type.isEmpty()){
        _db = &QSqlDatabase::addDatabase(type);
        if(type == "QSQLITE"){
            _db->setDatabaseName(ui->database_lineEdit->text());
        }
        else if(type == "QMYSQL"){
            _db->setHostName(ui->host_lineEdit->text());
            _db->setDatabaseName(ui->database_lineEdit->text());
            _db->setUserName(ui->user_lineEdit->text());
            _db->setPassword(ui->password_lineEdit->text());
        }
    }
    if(_db->open()){
        if(type == "QSQLITE"){
            QSqlQuery* query = new QSqlQuery(*_db);
            if(!query->exec(QString("SELECT id FROM users WHERE `username` LIKE \'%1\' AND `password` LIKE \'%2\'")
                            .arg(ui->user_lineEdit->text())
                            .arg(ui->password_lineEdit->text()))){
                showError(query->lastError().text(), 5);
                if(logger) logger->writeLog(Logger::Login, Logger::Users, tr("SQL Error").arg(query->lastError().text()));
               // qDebug() << "Last executed query: " << query->lastQuery();
                QApplication::restoreOverrideCursor();
                return;
            }
            if(!query->first()){
                showError(tr("Wrong username or password"));
                if(logger) logger->writeLog(Logger::Login, Logger::Users, tr("Wrong username (%1) or password").arg(ui->user_lineEdit->text()));
                QApplication::restoreOverrideCursor();
                return;
            }
            qApp->setProperty("user_id", query->value(0).toString());
            qApp->setProperty("data_path", ui->database_lineEdit->text());
            if(logger) logger->writeLog(Logger::Login);
        }

        qApp->setProperty("user", ui->user_lineEdit->text());
        qApp->setProperty("database", ui->database_lineEdit->text());
        this->saveToSettings();
        QApplication::restoreOverrideCursor();
        QDialog::accept();
    }
    else{
        showError(tr("Sql error: %1").arg(_db->lastError().text()));
        logger->writeLog(Logger::Error, Logger::Other, tr("Sql error:\n%1")
                         .arg(_db->lastError().text())
                         );
    }
    QApplication::restoreOverrideCursor();
}

void DbConnectDialog::setDatabaseTypes(const QString& type)
{
    if(type == "QSQLITE"){
        ui->host_lineEdit->hide();
        ui->hostname_label->hide();
        ui->database_lineEdit->show();
        ui->database_label->show();
        ui->database_toolButton->show();
        ui->user_lineEdit->show();
        ui->user_label->show();
        ui->password_lineEdit->show();
        ui->password_label->show();
    }
    else if(type == "QMYSQL"){
        ui->host_lineEdit->show();
        ui->hostname_label->show();
        ui->database_lineEdit->show();
        ui->database_label->show();
        ui->database_toolButton->hide();
        ui->user_lineEdit->show();
        ui->user_label->show();
        ui->password_lineEdit->show();
        ui->password_label->show();
    }
    else{
        ui->host_lineEdit->hide();
        ui->hostname_label->hide();
        ui->database_lineEdit->hide();
        ui->database_toolButton->hide();
        ui->database_label->hide();
        ui->user_lineEdit->hide();
        ui->user_label->hide();
        ui->password_lineEdit->hide();
        ui->password_label->hide();
    }
}

void DbConnectDialog::chooseSqliteDatabase()
{
    QString fileName = QFileDialog::getOpenFileName(this,
         tr("Open Database"), "", tr("Sqlite Files (*.sqlite);;All Files (*.*)"));
    if(!fileName.isEmpty()){
        ui->database_lineEdit->setText(fileName);
    }
}

void DbConnectDialog::showError(const QString &msg, const int secs)
{
    ui->error_label->setText(msg);
    ui->error_label->show();
    ui->cancel_toolButton->setDisabled(true);
    QTimer::singleShot(secs*1000, this, SLOT(hideError()));
}

void DbConnectDialog::loadFromSettings()
{
    //QDir appdir = qApp->applicationDirPath();
    //QSettings settings(appdir.filePath("Inventory.ini"), QSettings::IniFormat);
    QSettings settings(qApp->property("ini").toString(), QSettings::IniFormat);
    settings.beginGroup("Database");
    ui->host_lineEdit->setText(settings.value("Host", "").toString());
    //int dbt_idx = ui->dbtype_comboBox->findText(settings.value("Type", "QSQLITE").toString());
    //if( dbt_idx >= 0)
        //ui->dbtype_comboBox->setCurrentIndex(dbt_idx);
    ui->database_lineEdit->setText(settings.value("Database","").toString());
    ui->user_lineEdit->setText(settings.value("Username","").toString());
    ui->password_lineEdit->setText(settings.value("Password", "").toString());
    ui->remember_checkBox->setChecked(settings.value("Remember", false).toBool());
    settings.endGroup();
}

void DbConnectDialog::saveToSettings()
{
    //QDir appdir = qApp->applicationDirPath();
    //QSettings settings(appdir.filePath("Inventory.ini"), QSettings::IniFormat);
    QSettings settings(qApp->property("ini").toString(), QSettings::IniFormat);
    settings.beginGroup("Database");
    settings.setValue("Host", ui->host_lineEdit->text());
    //settings.setValue("Type", ui->dbtype_comboBox->currentText());
    settings.setValue("Database", ui->database_lineEdit->text());
    bool remember = ui->remember_checkBox->isChecked();
    settings.setValue("Remember", remember);
    settings.setValue("Username", (remember ? ui->user_lineEdit->text() : ""));
    settings.setValue("Password", (remember ? ui->password_lineEdit->text() : ""));
    settings.endGroup();
}

void DbConnectDialog::hideError()
{
    ui->error_label->hide();
    ui->cancel_toolButton->setDisabled(false);
}

void DbConnectDialog::newDatabase()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("New database"),
                                "",
                                tr("SQLITE Database (*.sqlite)"));
    if(!fileName.isEmpty()){
        QFile db_template(":/DB/inventory.sqlite");
        if(db_template.copy(fileName)){
            if(!QFile::setPermissions(fileName, QFile::WriteOther)){
                showError(tr("Can not set writable perrmissions"));
            }
            ui->database_lineEdit->setText(fileName);
            ui->user_lineEdit->setText("admin");
            ui->password_lineEdit->setText("admin");
        }
        else showError(tr("Database file creation error"));
    }
}

void DbConnectDialog::keyPressEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter){
        accept();
    }
    else
        return QDialog::keyPressEvent(e);
}

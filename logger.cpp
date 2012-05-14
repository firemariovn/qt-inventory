#include "logger.h"

#include <QtSql>
#include <QApplication>

Logger::Logger(QObject *parent) :
    QObject(parent)
{
    QString logpath(qApp->property("data_path").toString());
    if(logpath.isEmpty()) logpath =  QDir::home().filePath("Inventory.log");
    QFileInfo fi(logpath);
    QDir dir(fi.absoluteDir());
    fi.setFile( dir, "Inventory.log" );

    file.setFileName(fi.absoluteFilePath());
}

void Logger::writeLog(LoggedAction Action, ChangedTable Table, const QString& details)
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    QString action, table;
    switch(Action){
        case Logger::View: action = tr("View"); break;
        case Logger::Add: action = tr("Add"); break;
        case Logger::Edit: action = tr("Edit"); break;
        case Logger::Delete: action = tr("Delete"); break;
        case Logger::Login: action = tr("Login"); break;
        case Logger::Logout: action = tr("Logout"); break;
        case Logger::Info: action = tr("Info");
        case Logger::Error: action = tr("Error"); break;
        default: action = tr("Unknown"); break;
    }
    switch(Table){
        case Logger::Users: table = tr("Users"); break;
        case Logger::Rights: table = tr("Rights"); break;
        case Logger::Items: table = tr("Items"); break;
        case Logger::Item_status: table = tr("Item status"); break;
        case Logger::Item_types: table = tr("Item types"); break;
        case Logger::Operators: table = tr("Operators"); break;
        case Logger::Locations: table = tr("Locations"); break;
        case Logger::Allocations: table = tr("Allocations"); break;
        case Logger::Departments: table = tr("Departments"); break;
        case Logger::Logs: table = tr("Logs"); break;
        case Logger::Attachments: table = tr("Attachments"); break;
        case Logger::Scrap: table = tr("Scrap"); break;
        case Logger::Properties: table = tr("Properties"); break;
        case Logger::Other: table = tr("Other"); break;
        default: table = tr("Other"); break;
    }

    QSqlQuery* query = new QSqlQuery(QSqlDatabase::database());
    if(!query->exec(QString("INSERT INTO logs (`date`, `user_id`,`action`,`table`,`details`) "
                            "VALUES ('%1', '%2', '%3', '%4', '%5')")
                    .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"))
                    .arg(qApp->property("user_id").toInt())
                    .arg(action)
                    .arg(table)
                    .arg(details)
                    )){
        qDebug() << "Can not write log into database. Sql error: " << query->lastError().text();
        /*QString details(QString("%1, %2, %3, %4, %5")
                        .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"))
                        .arg(qApp->property("user_id").toInt())
                        .arg(action)
                        .arg(table)
                        .arg(details)
                        );
                        */
        QString log_details = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
        log_details.append(",");
        log_details.append(qApp->property("user_id").toString());
        log_details.append(",");
        log_details.append(action);
        log_details.append(",");
        log_details.append(table);
        log_details.append(",");
        log_details.append(details);
        log_details.append("|");
        log_details.append(query->lastError().text());

        writeTextLog(log_details);

        analizeError(query->lastError().text());
    }
    if(Action == Logger::Error){
        qDebug() << "Logged error: " << details;
    }
    QApplication::restoreOverrideCursor();
}

void Logger::writeTextLog(const QString &details)
{
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)){
        qDebug() << "Can not open log file: " << file.fileName();
        return;
    }

    QTextStream out(&file);
    out << details << "\n";
    file.close();
}

QString Logger::infoLog(QSqlQuery *query) const
{
    QString info;
    if(!query->first()){
        return info;
    }
    do{
        QSqlRecord record = query->record();
        info.append(infoLog(record));
    }
    while(query->next());
    return info;
}

QString Logger::infoLog(const QSqlRecord &record) const
{
    QString info;
    for(int i=0; i<record.count(); i++){
        info.append(QString("%1:%2|")
                    .arg(record.fieldName(i))
                    .arg(record.value(i).toString()));
    }
    return info;
}

void Logger::analizeError(const QString &error)
{
    if(error.contains("database is locked")){
        emit lockedDatabase();
    }
}

#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QFile>

class QSqlQuery;
class QSqlRecord;

class Logger : public QObject
{
    Q_OBJECT
public:
    explicit Logger(QObject *parent = 0);

    enum LoggedAction {
        View,
        Add,
        Edit,
        Delete,
        Login,
        Logout,
        Info,
        Error
    };
    enum ChangedTable {
        Users,
        Rights,
        Items,
        Item_status,
        Item_types,
        Locations,
        Departments,
        Operators,
        Allocations,
        Logs,
        Attachments,
        Scrap,
        Other
    };

    void writeLog(LoggedAction Action, ChangedTable Table = Logger::Other, const QString& details = QString::null);


    QString infoLog(QSqlQuery* query) const;
    QString infoLog(const QSqlRecord& record) const;

    void analizeError(const QString& error);


signals:
    void lockedDatabase();

private:
    void writeTextLog(const QString& details);
private:
    QFile file;

};

#endif // LOGGER_H

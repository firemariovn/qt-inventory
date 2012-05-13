#ifndef SCRAP_H
#define SCRAP_H

#include <QDialog>

namespace Ui {
class Scrap;
}

class QSqlRecord;

class Scrap : public QDialog
{
    Q_OBJECT
    
public:
    explicit Scrap(QWidget *parent = 0);
    ~Scrap();

    void setItem(const QSqlRecord& record);
    QString getReason() const;
    
private:
    Ui::Scrap *ui;
};

#endif // SCRAP_H

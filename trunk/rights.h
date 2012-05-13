#ifndef RIGHTS_H
#define RIGHTS_H

#include <QDialog>

class Logger;

namespace Ui {
    class Rights;
}

class Rights : public QDialog
{
    Q_OBJECT

public:
    explicit Rights(QWidget *parent = 0,  Qt::WindowFlags f = 0);
    ~Rights();

    inline void setId(const int id){_id=id;}
    inline int getId() const { return _id;}
    inline void setLogger(Logger* l) { logger = l; }

    void loadRights();
    void accept();

private:
    Ui::Rights *ui;
    Logger* logger;
    int _id;
};

#endif // RIGHTS_H

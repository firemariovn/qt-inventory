#ifndef USERS_H
#define USERS_H

#include <QDialog>

namespace Ui {
    class Users;
}

class Logger;

class Users : public QDialog
{
    Q_OBJECT

public:
    explicit Users(QWidget *parent = 0);
    ~Users();

    void accept();
    inline void setId(int id) { _id = id; }
    inline int getId() const { return _id; }
    void setEditMode(const bool edit);
    inline bool getEditMode() const { return _edit; }
    inline void setLogger(Logger* l) { logger = l; }
    void loadUser(int user_id);

private:
    void showError(const QString& msg, const int secs=3);
    bool checkPassword();
    bool saveUser();

    Ui::Users *ui;
    Logger* logger;
    int _id;
    bool _edit;
};

#endif // USERS_H

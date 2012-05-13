#ifndef CHANGEPASSWORD_H
#define CHANGEPASSWORD_H

#include <QDialog>

namespace Ui {
    class ChangePassword;
}

class ChangePassword : public QDialog
{
    Q_OBJECT

public:
    explicit ChangePassword(QWidget *parent = 0);
    ~ChangePassword();

    void accept();
    void loadUser();
    void loadUser(const int user_id);

private slots:
    bool saveUserPassword();
    void showError(const QString& msg, const int secs=3);
    void hideError();

private:
    Ui::ChangePassword *ui;
    QString password;
    int id;
};

#endif // CHANGEPASSWORD_H

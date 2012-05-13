#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDialog>

namespace Ui {
    class Settings;
}

class Settings : public QDialog
{
    Q_OBJECT

public:
    explicit Settings(QWidget *parent = 0);
    ~Settings();

    void accept();

signals:
    void showId(bool show);
    void tableUpdate(const QString& table);

private slots:
    void setBackgroundBrush();
    void setBoldFont(const bool set);

private:
    void loadSettings();
    void saveSettings();
    void loadAccessibleTables();
    Ui::Settings *ui;
};

#endif // SETTINGS_H

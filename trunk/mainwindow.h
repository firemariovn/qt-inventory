#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "interface.h"

class QSqlDatabase;
class QTranslator;

class TableForm;
class ChangePassword;
class Logger;
class Settings;
class Browser;
class ReferencesForm;

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = 0);
    ~MainWindow();

signals:
    void needUpdate(const QString& table);
public slots:
    inline void tableUpdate(const QString& table){ emit needUpdate(table); }
protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void about();
    void openDatabase();
    void openDatabase(QSqlDatabase* db);
    void showUsers();
    void showItems();
    void showLocations();
    void showOperators();
    void showDepartments();
    void showItemsTypes();
    void showItemsStatus();
    void showAllocations();
    void showScrap();
    void changePassword();
    void showLogs();
    void showSettings();
    void showSqlBrowser();
    void showId(bool show);
    void lockedDatabase();
    void searchItem(const int item_id, const QString& table);
    void switchLanguage(QAction *action);
    void showReferences();

    void on_actionLicensing_triggered();

private:
    void setUserRights();
    bool checkUserRights(const int idx, bool show_msg = true) const;
    bool checkDatabase() const;
    bool checkDatabaseTable(const QString& table, QStringList& fields) const;
    void setToolbar();
    void saveSettings();
    bool restoreSettings();
    void startWithTable();
    //void checkForUpdates();
    //bool isHigestVersion(const QStringList& latest);
    //void performUpdate(const QString& path);
    void createLanguageMenu();
    void loadLicenseInfo();
    bool loadUpdatePlugin();
    bool checkLicense();
    void updateDataTable();

    inline QSqlDatabase* getDB() {return db; }

    Ui::MainWindow* ui;
    TableForm* users_form;
    TableForm* items_form;
    TableForm* locations_form;
    TableForm* operators_form;
    TableForm* departments_form;
    TableForm* items_types_form;
    TableForm* items_status_form;
    TableForm* allocations_form;
    TableForm* scrap_form;
    TableForm* logs_form;
    Browser* sql_browser_form;
    ChangePassword* password_form;
    Logger* logger;
    Settings* settings_dialog;
    QSqlDatabase* db;
    QTranslator* translator;
    ReferencesForm* references_form;

    UpdMgrInterface* updmgr;
};

#endif // MAINWINDOW_H

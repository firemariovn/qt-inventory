#include "version.h"

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dbconnect.h"
#include "tableform.h"
#include "changepassword.h"
#include "logger.h"
#include "settings.h"
#include "browser.h"
#include "referencesform.h"

#include <QtGui>
#include <QtSql>
#include <QTimer>
#include <QMdiSubWindow>
#include <QtDebug>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    if(!this->restoreSettings()){
        qDebug() << "Can not restore program settings";
    }

    users_form = 0;
    items_form = 0;
    locations_form = 0;
    operators_form = 0;
    departments_form = 0;
    items_types_form = 0;
    items_status_form = 0;
    allocations_form = 0;
    scrap_form = 0;
    password_form = 0;
    logs_form = 0;
    settings_dialog = 0;
    sql_browser_form = 0;
    references_form = 0;

    translator = new QTranslator(this);
    qApp->installTranslator(translator);

    this->createLanguageMenu();

    db = new QSqlDatabase();
    logger = new Logger(this);

    connect(logger, SIGNAL(lockedDatabase()), this, SLOT(lockedDatabase()));

    this->openDatabase(db);

    connect(ui->actionUsers, SIGNAL(triggered()), this, SLOT(showUsers()));
    connect(ui->actionItems, SIGNAL(triggered()), this, SLOT(showItems()));
    connect(ui->actionLocations, SIGNAL(triggered()), this, SLOT(showLocations()));
    connect(ui->actionOperators, SIGNAL(triggered()), this, SLOT(showOperators()));
    connect(ui->actionDepartments, SIGNAL(triggered()), this, SLOT(showDepartments()));
    connect(ui->actionItems_types, SIGNAL(triggered()), this, SLOT(showItemsTypes()));
    connect(ui->actionItems_status, SIGNAL(triggered()), this, SLOT(showItemsStatus()));
    connect(ui->actionAllocations, SIGNAL(triggered()), this, SLOT(showAllocations()));
    connect(ui->actionLogs, SIGNAL(triggered()), this, SLOT(showLogs()));
    connect(ui->actionScrap, SIGNAL(triggered()), this, SLOT(showScrap()));
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(about()));
    connect(ui->actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    connect(ui->actionChange_password, SIGNAL(triggered()), this, SLOT(changePassword()));
    connect(ui->actionSettings, SIGNAL(triggered()), this, SLOT(showSettings()));
    connect(ui->actionSQL_Browser, SIGNAL(triggered()), this, SLOT(showSqlBrowser()));
    connect(ui->actionReferences, SIGNAL(triggered()), this, SLOT(showReferences()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::openDatabase()
{
    this->openDatabase(getDB());
}

void MainWindow::openDatabase(QSqlDatabase *db)
{
    DbConnectDialog* dlg = new DbConnectDialog(db, this);
    dlg->setLogger(logger);
    dlg->exec();
    if(!QSqlDatabase::database().isOpen()){
        this->setDisabled(true);
        QTimer::singleShot(1000, this, SLOT(close()));
    }
    else if(!checkDatabase()){
        logger->writeLog(Logger::Error, Logger::Other, tr("Wrong database!"));
        this->setDisabled(true);
        QTimer::singleShot(1000, this, SLOT(close()));
    }
    else{
        this->loadLicenseInfo();
        this->setUserRights();
        this->setToolbar();
        this->startWithTable();
        if (loadUpdatePlugin()){
            connect(updmgr->updmgrObject(), SIGNAL(blockUserInput(bool)), this, SLOT(setDisabled(bool)));
            updmgr->checkForUpdate("inventory",_STR_PRODUCT_VERSION);
        }
    }
}

void MainWindow::setUserRights()
{
    QSqlQuery* query = new QSqlQuery();
    if(!query->exec(QString("SELECT * FROM users_rights WHERE `user_id` LIKE \'%1\'")
                    .arg(qApp->property("user_id").toString()))){
        qDebug() << "SQL Error: " << query->lastError();
        qDebug() << "Last executed query: " << query->lastQuery();
        return;
    }
    if(!query->first()){
        qDebug() << "SQL Error: " << query->lastError();
        qDebug() << "Last executed query: " << query->lastQuery();
        return;
    }

    QSqlRecord record = query->record();
    QBitArray rights(record.count()-2);

    for(int n=0; n<record.count(); n++){
        if(record.fieldName(n) == "users_view") rights.setBit(0, query->value(n).toInt());
        else if(record.fieldName(n) == "users_edit") rights.setBit(1, query->value(n).toInt());
        else if(record.fieldName(n) == "users_delete") rights.setBit(2, query->value(n).toInt());
        else if(record.fieldName(n) == "items_view") rights.setBit(3, query->value(n).toInt());
        else if(record.fieldName(n) == "items_edit") rights.setBit(4, query->value(n).toInt());
        else if(record.fieldName(n) == "items_delete") rights.setBit(5, query->value(n).toInt());
        else if(record.fieldName(n) == "operators_view") rights.setBit(6, query->value(n).toInt());
        else if(record.fieldName(n) == "operators_edit") rights.setBit(7, query->value(n).toInt());
        else if(record.fieldName(n) == "operators_delete") rights.setBit(8, query->value(n).toInt());
        else if(record.fieldName(n) == "locations_view") rights.setBit(9, query->value(n).toInt());
        else if(record.fieldName(n) == "locations_edit") rights.setBit(10, query->value(n).toInt());
        else if(record.fieldName(n) == "locations_delete") rights.setBit(11, query->value(n).toInt());
        else if(record.fieldName(n) == "departments_view") rights.setBit(12, query->value(n).toInt());
        else if(record.fieldName(n) == "departments_edit") rights.setBit(13, query->value(n).toInt());
        else if(record.fieldName(n) == "departments_delete") rights.setBit(14, query->value(n).toInt());
        else if(record.fieldName(n) == "attachments_view") rights.setBit(15, query->value(n).toInt());
        else if(record.fieldName(n) == "attachments_edit") rights.setBit(16, query->value(n).toInt());
        else if(record.fieldName(n) == "attachments_delete") rights.setBit(17, query->value(n).toInt());
        else if(record.fieldName(n) == "item_types_view") rights.setBit(18, query->value(n).toInt());
        else if(record.fieldName(n) == "item_types_edit") rights.setBit(19, query->value(n).toInt());
        else if(record.fieldName(n) == "item_types_delete") rights.setBit(20, query->value(n).toInt());
        else if(record.fieldName(n) == "item_status_view") rights.setBit(21, query->value(n).toInt());
        else if(record.fieldName(n) == "item_status_edit") rights.setBit(22, query->value(n).toInt());
        else if(record.fieldName(n) == "item_status_delete") rights.setBit(23, query->value(n).toInt());        
        else if(record.fieldName(n) == "allocations_view") rights.setBit(24, query->value(n).toInt());
        else if(record.fieldName(n) == "allocations_edit") rights.setBit(25, query->value(n).toInt());
        else if(record.fieldName(n) == "allocations_add") rights.setBit(26, query->value(n).toInt());
        else if(record.fieldName(n) == "logs_view") rights.setBit(27, query->value(n).toInt());
        else if(record.fieldName(n) == "logs_delete") rights.setBit(28, query->value(n).toInt());
        else if(record.fieldName(n) == "sql_view") rights.setBit(29, query->value(n).toInt());
        else if(record.fieldName(n) == "sql_edit") rights.setBit(30, query->value(n).toInt());
        else if(record.fieldName(n) == "scrap_view") rights.setBit(31, query->value(n).toInt());
        else if(record.fieldName(n) == "scrap_edit") rights.setBit(32, query->value(n).toInt());
        else if(record.fieldName(n) == "scrap_delete") rights.setBit(33, query->value(n).toInt());
    }
    qApp->setProperty("user_rights", rights);

/*
    for(int i=0; i<rights.size();++i){
        qDebug() << QString("The right %1 is (%2)")
                    .arg(i)
                    .arg(record.fieldName(i));
    }
    */
}

void MainWindow::showUsers()
{
    if(!this->checkUserRights(0)) return;
    if(!checkLicense()) return;
    if(!users_form){
        users_form = new TableForm(ui->mdiArea);
        users_form->setLogger(logger);
        users_form->setTable("users");
        connect(users_form, SIGNAL(tableUpdate(QString)), this, SLOT(tableUpdate(QString)));
        connect(users_form, SIGNAL(searchItem(int,QString)), this, SLOT(searchItem(int,QString)));
        connect(this, SIGNAL(needUpdate(QString)), users_form, SLOT(needUpdate(QString)));
        QMdiSubWindow *subWindowUsers = new QMdiSubWindow;
        subWindowUsers->setWindowIcon(QIcon(":/Icons/icons/User.png"));
        subWindowUsers->setWidget(users_form);
        ui->mdiArea->addSubWindow(subWindowUsers);
        subWindowUsers->showMaximized();
        logger->writeLog(Logger::View, Logger::Users);
    }
    else{
        users_form->showMaximized();
    }
}

void MainWindow::showItems()
{
    if(!this->checkUserRights(3)) return;
    if(!checkLicense()) return;
    if(!items_form){
        items_form = new TableForm(ui->mdiArea);
        items_form->setLogger(logger);
        items_form->setTable("items");
        connect(items_form, SIGNAL(tableUpdate(QString)), this, SLOT(tableUpdate(QString)));
        connect(items_form, SIGNAL(searchItem(int,QString)), this, SLOT(searchItem(int,QString)));
        connect(this, SIGNAL(needUpdate(QString)), items_form, SLOT(needUpdate(QString)));
        QMdiSubWindow *subWindowItems = new QMdiSubWindow;
        subWindowItems->setWindowIcon(QIcon(":/Icons/icons/Computer.png"));
        subWindowItems->setWidget(items_form);
        ui->mdiArea->addSubWindow(subWindowItems);
        subWindowItems->showMaximized();
        logger->writeLog(Logger::View, Logger::Items);
    }
    else{
        items_form->showMaximized();
    }
}

void MainWindow::showItemsTypes()
{
    if(!this->checkUserRights(18)) return;
    if(!checkLicense()) return;
    if(!items_types_form){
        items_types_form = new TableForm(ui->mdiArea);
        items_types_form->setLogger(logger);
        items_types_form->setTable("item_types");
        connect(items_types_form, SIGNAL(tableUpdate(QString)), this, SLOT(tableUpdate(QString)));
        connect(items_types_form, SIGNAL(searchItem(int,QString)), this, SLOT(searchItem(int,QString)));
        connect(this, SIGNAL(needUpdate(QString)), items_types_form, SLOT(needUpdate(QString)));
        QMdiSubWindow *subWindowItemsTypes = new QMdiSubWindow;
        subWindowItemsTypes->setWindowIcon(QIcon(":/Icons/icons/Sitemap.png"));
        subWindowItemsTypes->setWidget(items_types_form);
        ui->mdiArea->addSubWindow(subWindowItemsTypes);
        subWindowItemsTypes->showMaximized();
        logger->writeLog(Logger::View, Logger::Item_types);
    }
    else{
        items_types_form->showMaximized();
    }
}

void MainWindow::showItemsStatus()
{
    if(!this->checkUserRights(21)) return;
    if(!checkLicense()) return;
    if(!items_status_form){
        items_status_form = new TableForm(ui->mdiArea);
        items_status_form->setLogger(logger);
        items_status_form->setTable("item_status");
        connect(items_status_form, SIGNAL(tableUpdate(QString)), this, SLOT(tableUpdate(QString)));
        connect(items_status_form, SIGNAL(searchItem(int,QString)), this, SLOT(searchItem(int,QString)));
        connect(this, SIGNAL(needUpdate(QString)), items_status_form, SLOT(needUpdate(QString)));
        QMdiSubWindow *subWindowItemsStatus = new QMdiSubWindow;
        subWindowItemsStatus->setWindowIcon(QIcon(":/Icons/icons/Info2.png"));
        subWindowItemsStatus->setWidget(items_status_form);
        ui->mdiArea->addSubWindow(subWindowItemsStatus);
        subWindowItemsStatus->showMaximized();
        logger->writeLog(Logger::View, Logger::Item_status);
    }
    else{
        items_status_form->showMaximized();
    }
}

void MainWindow::showLocations()
{
    if(!this->checkUserRights(9)) return;
    if(!checkLicense()) return;
    if(!locations_form){
        locations_form = new TableForm(ui->mdiArea);
        locations_form->setLogger(logger);
        locations_form->setTable("locations");
        connect(locations_form, SIGNAL(tableUpdate(QString)), this, SLOT(tableUpdate(QString)));
        connect(this, SIGNAL(needUpdate(QString)), locations_form, SLOT(needUpdate(QString)));
        connect(locations_form, SIGNAL(searchItem(int,QString)), this, SLOT(searchItem(int,QString)));
        QMdiSubWindow *subWindowLocations = new QMdiSubWindow;
        subWindowLocations->setWindowIcon(QIcon(":/Icons/icons/Home.png"));
        subWindowLocations->setWidget(locations_form);
        ui->mdiArea->addSubWindow(subWindowLocations);
        subWindowLocations->showMaximized();
        logger->writeLog(Logger::View, Logger::Locations);
    }
    else{
        locations_form->showMaximized();
    }
}

void MainWindow::showDepartments()
{
    if(!this->checkUserRights(12)) return;
    if(!checkLicense()) return;
    if(!departments_form){
        departments_form = new TableForm(ui->mdiArea);
        departments_form->setLogger(logger);
        departments_form->setTable("departments");
        connect(departments_form, SIGNAL(tableUpdate(QString)), this, SLOT(tableUpdate(QString)));
        connect(this, SIGNAL(needUpdate(QString)), departments_form, SLOT(needUpdate(QString)));
        QMdiSubWindow *subWindowDepartments = new QMdiSubWindow;
        subWindowDepartments->setWindowIcon(QIcon(":/Icons/icons/Flag.png"));
        subWindowDepartments->setWidget(departments_form);
        ui->mdiArea->addSubWindow(subWindowDepartments);
        subWindowDepartments->showMaximized();
        logger->writeLog(Logger::View, Logger::Departments);
    }
    else{
        departments_form->showMaximized();
    }
}

void MainWindow::showScrap()
{
    if(!this->checkUserRights(31)) return;
    if(!checkLicense()) return;
    if(!scrap_form){
        scrap_form = new TableForm(ui->mdiArea);
        scrap_form->setLogger(logger);
        scrap_form->setTable("scrap");
        connect(scrap_form, SIGNAL(tableUpdate(QString)), this, SLOT(tableUpdate(QString)));
        connect(this, SIGNAL(needUpdate(QString)), scrap_form, SLOT(needUpdate(QString)));
        QMdiSubWindow *subWindowScrap = new QMdiSubWindow;
        subWindowScrap->setWindowIcon(QIcon(":/Icons/icons/Trash.png"));
        subWindowScrap->setWidget(scrap_form);
        ui->mdiArea->addSubWindow(subWindowScrap);
        subWindowScrap->showMaximized();
        logger->writeLog(Logger::View, Logger::Scrap);
    }
    else{
        scrap_form->showMaximized();
    }
}

void MainWindow::setToolbar()
{
    ui->mainToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    ui->mainToolBar->addAction(ui->actionExit);
    ui->mainToolBar->addSeparator();
    if(this->checkUserRights(0, false)) ui->mainToolBar->addAction(ui->actionUsers);
    else{
        ui->actionUsers->setDisabled(true);
    }
    if(this->checkUserRights(3, false)){
        ui->mainToolBar->addAction(ui->actionItems);
        ui->mainToolBar->addAction(ui->actionReferences);
    }
    else {
        ui->actionItems->setDisabled(true);
        ui->actionReferences->setDisabled(true);
    }
    if(this->checkUserRights(18, false)) ui->mainToolBar->addAction(ui->actionItems_types);
    else ui->actionItems_types->setDisabled(true);
    if(this->checkUserRights(21, false)) ui->mainToolBar->addAction(ui->actionItems_status);
    else ui->actionItems_status->setDisabled(true);
    if(this->checkUserRights(9, false)) ui->mainToolBar->addAction(ui->actionLocations);
    else ui->actionLocations->setDisabled(true);
    if(this->checkUserRights(12, false)) ui->mainToolBar->addAction(ui->actionDepartments);
    else ui->actionDepartments->setDisabled(true);
    if(this->checkUserRights(6, false)) ui->mainToolBar->addAction(ui->actionOperators);
    else ui->actionOperators->setDisabled(true);
    if(this->checkUserRights(31, false)) ui->mainToolBar->addAction(ui->actionScrap);
    else ui->actionScrap->setDisabled(true);
    if(this->checkUserRights(24, false)){
        ui->mainToolBar->addSeparator();
        ui->mainToolBar->addAction(ui->actionAllocations);
    }
    else ui->actionAllocations->setDisabled(true);
    if(this->checkUserRights(27, false)){
        ui->mainToolBar->addSeparator();
        ui->mainToolBar->addAction(ui->actionLogs);
    }
    else{
        ui->actionLogs->setDisabled(true);
    }
    if(this->checkUserRights(29, false)){
        ui->mainToolBar->addSeparator();
        ui->mainToolBar->addAction(ui->actionSQL_Browser);
    }
    else ui->actionSQL_Browser->setDisabled(true);

    QSqlQuery* query = new QSqlQuery();
    if(!query->exec(QString("SELECT * FROM users WHERE `id`=%1")
                    .arg(qApp->property("user_id").toInt()))){
        qDebug() << "SQL Error: " << query->lastError();
        qDebug() << "Last executed query: " << query->lastQuery();
        return;
    }
    if(!query->first()){
        qDebug() << "SQL Error: " << query->lastError();
        qDebug() << "Last executed query: " << query->lastQuery();
        return;
    }
    QSqlRecord record = query->record();

    QString user = QString("%1 %2")
            .arg(record.value("firstname").toString())
            .arg(record.value("lastname").toString())
            ;
    if(user.count() < 2){
        user = record.value("username").toString();
    }
    setWindowTitle(windowTitle().append(" - "+user));
}

void MainWindow::closeEvent(QCloseEvent *event)
 {
    QList<QMdiSubWindow *> wlist = ui->mdiArea->subWindowList();
    for (int i = 0; i < wlist.size(); ++i) {
        if (!wlist.at(i)->close()){
            event->ignore();
            return;
        }
    }

    this->saveSettings();
    if(logger) logger->writeLog(Logger::Logout);
    event->accept();
 }

bool MainWindow::checkUserRights(const int idx, bool show_msg) const
{
    QBitArray rights(qApp->property("user_rights").toBitArray());
    if(!rights[idx]){
        //qDebug() << "User has no rights!";
        if(show_msg){
            QMessageBox msgBox;
            msgBox.setText(tr("You are not authorized to perform this operation!\nPlease contact your system administrator."));
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.exec();
        }
        return false;
    }
    else {
        return true;
    }
}

void MainWindow::showOperators()
{
    if(!this->checkUserRights(6)) return;
    if(!checkLicense()) return;
    if(!operators_form){
        operators_form = new TableForm(ui->mdiArea);
        operators_form->setLogger(logger);
        operators_form->setTable("operators");
        connect(operators_form, SIGNAL(tableUpdate(QString)), this, SLOT(tableUpdate(QString)));
        connect(operators_form, SIGNAL(searchItem(int,QString)), this, SLOT(searchItem(int,QString)));
        connect(this, SIGNAL(needUpdate(QString)), operators_form, SLOT(needUpdate(QString)));
        QMdiSubWindow *subWindowOperators = new QMdiSubWindow;
        subWindowOperators->setWindowIcon(QIcon(":/Icons/icons/Man.png"));
        subWindowOperators->setWidget(operators_form);
        ui->mdiArea->addSubWindow(subWindowOperators);
        subWindowOperators->showMaximized();
        if(logger) logger->writeLog(Logger::View, Logger::Operators);
    }
    else{
        operators_form->showMaximized();
    }
}

void MainWindow::showAllocations()
{
    if(!this->checkUserRights(24)) return;
    if(!checkLicense()) return;
    if(!allocations_form){
        allocations_form = new TableForm(ui->mdiArea);
        allocations_form->setLogger(logger);
        allocations_form->setTable("allocations");
        connect(allocations_form, SIGNAL(tableUpdate(QString)), this, SLOT(tableUpdate(QString)));
        connect(allocations_form, SIGNAL(searchItem(int,QString)), this, SLOT(searchItem(int,QString)));
        connect(this, SIGNAL(needUpdate(QString)), allocations_form, SLOT(needUpdate(QString)));
        QMdiSubWindow *subWindowAllocations = new QMdiSubWindow;
        subWindowAllocations->setWindowIcon(QIcon(":/Icons/icons/Puzzle.png"));
        subWindowAllocations->setWidget(allocations_form);
        ui->mdiArea->addSubWindow(subWindowAllocations);
        subWindowAllocations->showMaximized();
        if(logger) logger->writeLog(Logger::View, Logger::Allocations);
    }
    else{
        allocations_form->showMaximized();
    }
}

void MainWindow::about()
{
    QString msg(tr("Version: %1\nAuthor: Venelin Georgiev\nPleven, Bulgaria\nE-mail: veni_georgiev@mail.bg\n\n")
                .arg(_STR_PRODUCT_VERSION));

    if(!qApp->property("licensed").toBool()){

        msg.append(tr("Unregistered version!"));
    }
    else if(!qApp->property("license_actual").toBool()){
        msg.append(tr("Expired license!"));
    }
    else{
        msg.append(tr("Registered version.\nUser: %1.\nLicense expires on: %2 y.")
                   .arg(qApp->property("licensed_user").toString())
                   .arg(qApp->property("license_to_date").toDate().toString("dd.MM.yyyy")
                   ));
    }
    QMessageBox::about( this, tr("About Inventory"), msg) ;
}

bool MainWindow::checkDatabase()
{
    QStringList neededFields;
    neededFields << "id" << "status_id" << "IP" << "note" << "location_id" << "model" << "manufacturer";
    neededFields << "serialno" << "type_id" << "number" << "operator_id";
    if(!checkDatabaseTable("items", neededFields)) return false;

    neededFields << "id" << "name" << "department_id" << "note";
    if(!checkDatabaseTable("operators", neededFields)) return false;

    neededFields << "id" << "location";
    if(!checkDatabaseTable("locations", neededFields)) return false;

    neededFields << "id" << "date" << "item_id" << "from_location_id";
    neededFields << "to_location_id" << "from_operator_id" << "to_operator_id" << "note";
    if(!checkDatabaseTable("allocations", neededFields)) return false;

    neededFields << "id" << "status";
    if(!checkDatabaseTable("item_status", neededFields)) return false;

    neededFields << "id" << "username" << "password" << "firstname" << "lastname" << "email";
    if(!checkDatabaseTable("users", neededFields)) return false;

    neededFields << "id" << "department";
    if(!checkDatabaseTable("departments", neededFields)) return false;

    neededFields << "id" << "item_id" << "tablename" << "filename";
    if(!checkDatabaseTable("attachments", neededFields)) return false;

    neededFields << "id" << "type";
    if(!checkDatabaseTable("item_types", neededFields)) return false;

    neededFields << "id" << "date" << "user_id" << "action" << "table" << "details";
    if(!checkDatabaseTable("logs", neededFields)) return false;

    neededFields << "id" << "user_id";
    neededFields << "allocations_add" << "allocations_edit" << "allocations_view";
    neededFields << "item_status_delete" << "item_status_edit" << "item_status_view";
    neededFields << "departments_delete" << "departments_edit" << "departments_view";
    neededFields << "item_types_delete" << "item_types_edit" << "item_types_view";
    neededFields << "attachments_delete" << "attachments_edit" << "attachments_view";
    neededFields << "operators_delete" << "operators_edit" << "operators_view";
    neededFields << "users_view" << "users_edit" << "users_delete";
    neededFields << "items_view" << "items_edit" << "items_delete";
    neededFields << "locations_view" << "locations_edit" << "locations_delete";
    neededFields << "logs_view" << "logs_delete";
    neededFields << "sql_view" << "sql_edit";
    neededFields << "scrap_view" << "scrap_edit" << "scrap_delete";
    if(!checkDatabaseTable("users_rights", neededFields)){
        return false;
    }
    neededFields << "id" << "item_id" << "type" << "number" << "location" << "IP" << "reason";
    neededFields << "operator" << "manufacturer" << "model" << "serialno" << "note" << "allocations_data";
    if(!checkDatabaseTable("scrap", neededFields)) return false;
    neededFields << "id" << "type_id" << "property";
    if(!checkDatabaseTable("properties", neededFields)){
        updateDataTable("properties");
        if(!checkDatabaseTable("properties", neededFields)) return false;
    }
    neededFields << "id" << "property_id" << "item_id" << "value";
    if(!checkDatabaseTable("item_properties", neededFields)){
        updateDataTable("item_properties");
        if(!checkDatabaseTable("item_properties", neededFields)) return false;
    }
    return true;
}

bool MainWindow::checkDatabaseTable(const QString &table, QStringList &fields) const
{
    QSqlQuery* query = new QSqlQuery();
    if(!query->exec(QString("PRAGMA table_info( %1 )").arg(table))){
        qDebug() << "SQL error: " << query->lastError();
        qDebug() << "Last executed query: " << query->lastQuery();
        return false;
    }
    while(query->next()){
        fields.removeOne(query->record().value("name").toString());
    }
    if(fields.count() > 0){
        QMessageBox::critical( 0, tr("Wrong database!"), tr("Missed fields in table <b>%1</b> detected:<br><i>%2</i>")
                               .arg(table)
                               .arg(fields.join("|")));
        if(logger) logger->writeLog(Logger::Info, Logger::Other, tr("Missed fields in table %1 detected: %2")
                                                                                             .arg(table)
                                                                                             .arg(fields.join("|")));
        return false;
    }

    return true;
}

void MainWindow::changePassword()
{
    if(!password_form){
        password_form = new ChangePassword(this);
    }
    password_form->loadUser();
    if(password_form->exec() == QDialog::Accepted){
        logger->writeLog(Logger::Edit, Logger::Users, tr("User has changed his password."));
        emit tableUpdate("users");
    }
}

void MainWindow::saveSettings()
{
    //QDir appdir = qApp->applicationDirPath();
    //QSettings settings(appdir.filePath("Inventory.ini"), QSettings::IniFormat);
    QSettings settings(qApp->property("ini").toString(), QSettings::IniFormat);
    settings.beginGroup("InventoryWindow");
    settings.setValue("Geometry", this->saveGeometry());
    settings.endGroup();
    settings.beginGroup("InventorySettings");
    settings.setValue("Locale", qApp->property("locale").toString());
    settings.endGroup();
}

bool MainWindow::restoreSettings()
{
    //QDir appdir = qApp->applicationDirPath();
    //QSettings settings(appdir.filePath("Inventory.ini"), QSettings::IniFormat);
    QSettings settings(qApp->property("ini").toString(), QSettings::IniFormat);
    settings.beginGroup("InventorySettings");
    qApp->setProperty("locale", settings.value("Locale", "en").toString());
    settings.beginGroup("View");
       qApp->setProperty("show_id", settings.value("ShowIdEverywhere", false).toBool());
       settings.beginGroup("MarkingRows");
       qApp->setProperty("marking_rows", settings.value("MarkingEnable", false).toBool());
       qApp->setProperty("marking_bold_font", settings.value("MarkingBold", true).toBool());
       qApp->setProperty("marking_selected_brush", settings.value("SelectedBackgroundBrush"));
       qApp->setProperty("marking_unselected_brush", settings.value("UnselectedBackgroundBrush"));
       settings.endGroup();
    settings.endGroup();
    settings.endGroup();
    settings.beginGroup("InventoryWindow");
    bool ok = this->restoreGeometry(settings.value("Geometry").toByteArray());
    settings.endGroup();

    return ok;
}

void MainWindow::showLogs()
{
    if(!this->checkUserRights(27)) return;
    if(!checkLicense()) return;
    if(!logs_form){
        logs_form = new TableForm(ui->mdiArea);
        logs_form->setLogger(logger);
        logs_form->setTable("logs");
        connect(logs_form, SIGNAL(tableUpdate(QString)), this, SLOT(tableUpdate(QString)));
        connect(this, SIGNAL(needUpdate(QString)), logs_form, SLOT(needUpdate(QString)));
        QMdiSubWindow *subWindowLogs = new QMdiSubWindow;
        subWindowLogs->setWindowIcon(QIcon(":/Icons/icons/Write2.png"));
        subWindowLogs->setWidget(logs_form);
        ui->mdiArea->addSubWindow(subWindowLogs);
        subWindowLogs->showMaximized();
        if(logger) logger->writeLog(Logger::View, Logger::Logs);
    }
    else{
        logs_form->showMaximized();
    }
}

void MainWindow::showSettings()
{
    if(!settings_dialog){
        settings_dialog = new Settings(this);
        connect(settings_dialog, SIGNAL(tableUpdate(QString)), this, SLOT(tableUpdate(QString)));
        connect(settings_dialog, SIGNAL(showId(bool)), this, SLOT(showId(bool)));
    }
    settings_dialog->exec();
}

void MainWindow::showId(bool show)
{
    if(users_form){
        users_form->showHeaderId(show);
    }
    if(allocations_form){
        allocations_form->showHeaderId(show);
    }
    if(items_form){
        items_form->showHeaderId(show);
    }
    if(locations_form){
        locations_form->showHeaderId(show);
    }
    if(operators_form){
        operators_form->showHeaderId(show);
    }
    if(departments_form){
        departments_form->showHeaderId(show);
    }
    if(items_types_form){
        items_types_form->showHeaderId(show);
    }
    if(items_status_form){
        items_status_form->showHeaderId(show);
    }
    if(logs_form){
        logs_form->showHeaderId(show);
    }
    if(scrap_form){
        scrap_form->showHeaderId(show);
    }
}

void MainWindow::startWithTable()
{
    //QDir appdir = qApp->applicationDirPath();
    //QSettings settings(appdir.filePath("Inventory.ini"), QSettings::IniFormat);
    QSettings settings(qApp->property("ini").toString(), QSettings::IniFormat);
    settings.beginGroup("InventorySettings");
    settings.beginGroup("Startup");
    bool ok = settings.value("AutostartTable", false).toBool();
    QString table = settings.value("AutostartTableName").toString();
    settings.endGroup();
    settings.endGroup();

    if(!ok || table.isEmpty()) return;
    if(table == tr("Users")) showUsers();
    else if(table == tr("Items")) showItems();
    else if(table == tr("Operators")) showOperators();
    else if(table == tr("Locations")) showLocations();
    else if(table == tr("Departments")) showDepartments();
    else if(table == tr("Allocations")) showAllocations();
}

void MainWindow::lockedDatabase()
{
    QMessageBox::critical( this, tr("Database"), tr("Database is <b>locked</b> from another user of the system.\n"
                                                    "You can not edit data.\n"
                                                    "If the problem persists, contact your system administrator."));
}

void MainWindow::searchItem(const int item_id, const QString &table)
{
    if(table == "items"){
        showItems();
        items_form->findItem(item_id);
    }
    if(table == "allocations") {
        showAllocations();
        allocations_form->findItem(item_id);
    }
}

/****
void MainWindow::checkForUpdates()
{
    QSettings settings(qApp->property("ini").toString(), QSettings::IniFormat);
    settings.beginGroup("InventorySettings");
    settings.beginGroup("Startup");
    bool check = settings.value("CheckForUpdates", true).toBool();
    settings.endGroup();
    settings.endGroup();
    if(!check) return;
    QString update(qApp->property("data_path").toString());
    if(update.isEmpty()) return;
    QFileInfo fi(update);
    QDir dir(fi.absoluteDir());
    fi.setFile( dir, "version" );
    QFile file(fi.absoluteFilePath());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        logger->writeLog(Logger::Error, Logger::Other,
                         tr("Can not open update file:\n%1").arg(fi.absoluteFilePath())
                         );
        return;
    }
    QTextStream out(&file);
    QString version = out.readAll();
    file.close();
    QStringList latest =   version.split(',');
    if(!isHigestVersion(latest)) return;
    else performUpdate(dir.absoluteFilePath(latest.at(4)));
}

bool MainWindow::isHigestVersion(const QStringList &latest)
{
    QStringList current = QString(_STR_FILE_VERSION).split(',');

    if(latest.count() >= 4 && current.count() >= 4 ){
        if(current.at(0).toInt() < latest.at(0).toInt()) return true;
        if(current.at(1).toInt() < latest.at(1).toInt()) return true;
        if(current.at(2).toInt() < latest.at(2).toInt()) return true;
        if(current.at(3).toInt() < latest.at(3).toInt()) return true;
    }
    return false;
}


void MainWindow::performUpdate(const QString &path)
{
    int ret = QMessageBox::question(0, tr("Inventory"),
                                    tr("There is a new, more recent version of this software.\n"
                                       "Do you want to update it?"),
                                        QMessageBox::Ok | QMessageBox::Cancel);
    if(ret == QMessageBox::Cancel) return;
    logger->writeLog(Logger::Info, Logger::Other,
                     tr("Perform program update:\n%1").arg(path)
                     );
    QUrl url;

    if (path.startsWith("\\\\") || path.startsWith("//"))
        url.setUrl(QDir::toNativeSeparators(path));
    else
        url = QUrl::fromLocalFile(path);
    if(!QDesktopServices::openUrl(url)){
        if(logger) logger->writeLog(Logger::Error, Logger::Other,
                                    tr("Error with program update. The url path:\n%1")
                                    .arg(path)
                                    );
        return;
    }
    this->setDisabled(true);
    QTimer::singleShot(2000, this, SLOT(close()));
}
*/

void MainWindow::showSqlBrowser()
{
    if(!this->checkUserRights(29)) return;
    if(!checkLicense()) return;
    if(!sql_browser_form){
        sql_browser_form = new Browser(ui->mdiArea);
        sql_browser_form->addConnection();

        connect(sql_browser_form, SIGNAL(statusMessage(QString)), statusBar(), SLOT(showMessage(QString)));

        QMdiSubWindow *subWindowSqlBrowser = new QMdiSubWindow;
        subWindowSqlBrowser->setWindowIcon(QIcon(":/Icons/icons/Database.png"));
        subWindowSqlBrowser->setWidget(sql_browser_form);
        ui->mdiArea->addSubWindow(subWindowSqlBrowser);
        subWindowSqlBrowser->showMaximized();
        if(logger) logger->writeLog(Logger::View, Logger::Other, tr("User open SQL Browser"));
    }
    else{
        sql_browser_form->showMaximized();
    }
}

void MainWindow::createLanguageMenu()
{
    QString language;
    language = tr("Language");
    QActionGroup* languageActionGroup = new QActionGroup(this);
    connect(languageActionGroup, SIGNAL(triggered(QAction *)),
            this, SLOT(switchLanguage(QAction *)));

    QDir qmDir(":/Languages");
    QStringList fileNames =
            qmDir.entryList(QStringList("Inventory_*.qm"));

    for (int i = 0; i < fileNames.size(); ++i) {
        QString locale = fileNames[i];
        locale.remove(0, locale.indexOf('_') + 1);
        locale.chop(3);

        QTranslator _translator;
        _translator.load(fileNames[i], qmDir.absolutePath());
        language = _translator.translate("MainWindow","Language");

        QAction *action = new QAction(tr("&%1 %2")
                                      .arg(i + 1).arg(language), this);
        action->setCheckable(true);
        action->setData(locale);

        ui->menuLanguages->addAction(action);
        languageActionGroup->addAction(action);

        if (qApp->property("locale") == locale){
                action->setChecked(true);
                action->trigger();
        }
    }
}

void MainWindow::switchLanguage(QAction *action)
{
    QString locale = action->data().toString();
    qApp->setProperty("locale", locale);
    QString qmPath = (":/Languages");

    translator->load("Inventory_" + locale, qmPath);
    ui->retranslateUi(this);
}

void MainWindow::loadLicenseInfo()
{
    QDir appdir = qApp->applicationDirPath();
    QString fileName = appdir.absoluteFilePath("inventory.lic");
    if(fileName.isEmpty()) return;
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        qApp->setProperty("licensed", false);
        return;
    }

    QByteArray line = file.readAll();
    file.close();
    if(line.isEmpty()){
        qApp->setProperty("licensed", false);
        return;
    }
    QStringList list( QString(QByteArray::fromHex(line).data()).split('\n'));
    if(list.size() != 4){
        qApp->setProperty("licensed", false);
        return;
    } //invalid license data
    QString app(list.at(0).toLocal8Bit().constData());
    if(app != "Inventory"){
        qApp->setProperty("licensed", false);
        return;
    }
    else qApp->setProperty("licensed", true);
    qApp->setProperty("licensed_user", list.at(3).toLocal8Bit().constData());
    QDate from(QDate::fromString(list.at(1).toLocal8Bit().constData(), "dd:MM:yyyy"));
    QDate to(QDate::fromString(list.at(2).toLocal8Bit().constData(), "dd:MM:yyyy"));

    if(!from.isValid() || !to.isValid()){
        qApp->setProperty("license_actual", false);
        return;
    }
    if(QDate::currentDate() < from || QDate::currentDate() > to){
        qApp->setProperty("license_actual", false);
        return;
    }
    qApp->setProperty("license_actual", true);
    qApp->setProperty("license_to_date", to);
    ui->menuHelp->removeAction(ui->actionLicensing);
}


void MainWindow::on_actionLicensing_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                    "",
                                                    tr("Licenses (*.lic)"));
    if(fileName.isEmpty()) return;
    QFile file(fileName);
    QString err;
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)){
        QByteArray line = file.readAll();
        if(!line.isEmpty()){
            QStringList list( QString(QByteArray::fromHex(line).data()).split('\n'));
            if(list.size() == 4){
                QString app(list.at(0).toLocal8Bit().constData());
                if(app == "Inventory"){
                    QDate from(QDate::fromString(list.at(1).toLocal8Bit().constData(), "dd:MM:yyyy"));
                    QDate to(QDate::fromString(list.at(2).toLocal8Bit().constData(), "dd:MM:yyyy"));
                    if(from.isValid() || to.isValid()){
                        if(QDate::currentDate() > from || QDate::currentDate() < to){
                            QDir appdir = qApp->applicationDirPath();
                            QString toFileName = appdir.absoluteFilePath("inventory.lic");
                            QString toFileNameBak = toFileName+".bak";
                            if(QFile::exists(toFileName)){
                                if(!QFile::copy(toFileName, toFileNameBak)){
                                    err.append(tr("Old license file backup I/O error!\n"));
                                    QMessageBox msgBox;
                                    msgBox.setText(tr("Invalid license file and / or license information! \n\n%1")
                                                  .arg(err)
                                                   );
                                    msgBox.exec();
                                    return;
                                }
                            }
                            if(QFile::exists(toFileName)){
                                if(!QFile::remove(toFileName)){
                                    err.append(tr("Old license file remove error!\n"));
                                    QMessageBox msgBox;
                                    msgBox.setText(tr("Invalid license file and / or license information! \n\n%1")
                                                  .arg(err)
                                                   );
                                    msgBox.exec();
                                    return;
                                }
                            }
                            if(QFile::copy(fileName, toFileName)){
                                loadLicenseInfo();
                                QMessageBox msgBox;
                                msgBox.setText(tr("The new license information was taken."));
                                msgBox.exec();
                                QFile::remove(toFileNameBak);
                                return;
                            }
                            else {
                                err.append(tr("License file copy error!\n"));
                                QFile::copy(toFileNameBak, toFileName);
                            }
                        }
                        else err.append(tr("Expired license file!\n"));
                    }
                    else err.append(tr("Invalid period content!\n"));
                }
                else err.append(tr("This license file is for another program!\n"));
            }
            else err.append(tr("Invalid license content!\n"));
        }
        else err.append(tr("Empty license file!\n"));
    }
    QMessageBox msgBox;
    msgBox.setText(tr("Invalid license file and / or license information! \n\n%1")
                   .arg(err)
                   );
    msgBox.exec();
}

bool MainWindow::checkLicense()
{
    /*license restrictions*/
    if(!qApp->property("licensed").toBool()){
        if(logger) logger->writeLog(Logger::Error, Logger::Other, tr("License missing warning"));
        QMessageBox::critical ( this, tr("License missing"), tr("You are working with unregistered version of this program!\n"
                                                                "Please contact your system administrator!"));
        return false;
    }
    else if(!qApp->property("license_actual").toBool()){
        if(logger) logger->writeLog(Logger::Error, Logger::Other, tr("License expired warning"));
        QMessageBox::critical ( this, tr("License expired"), tr("The period for using on your license has expired!\n"
                                                                "Please contact your system administrator!"));
        return false;
    }
    return true;
}

bool MainWindow::loadUpdatePlugin()
{
    QDir pluginsDir(qApp->applicationDirPath());
    foreach (QString fileName, pluginsDir.entryList(QDir::Files)) {
        QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(fileName));
        QObject *plugin = pluginLoader.instance();
        if (plugin) {
            updmgr = qobject_cast<UpdMgrInterface *>(plugin);
            if (updmgr) return true;
        }
     }
    return false;
}

void MainWindow::updateDataTable(const QString& table)
{
    if(table.isEmpty()) return;
    QSqlQuery* query = new QSqlQuery();
    if(table == "properties"){
        if(!query->exec(QString("CREATE TABLE IF NOT EXISTS "
                                "main.properties "
                                "(`id` INTEGER PRIMARY KEY  AUTOINCREMENT  NOT NULL , `type_id` INTEGER, `property` VARCHAR)"
                                ))){
            qDebug() << "SQL Error: " << query->lastError();
            logger->writeLog(Logger::Error, Logger::Other, query->lastError().text());
            return;
        }
    }
    else if(table == "item_properties"){
        if(!query->exec(QString("CREATE TABLE IF NOT EXISTS "
                                "main.item_properties "
                                "(`id` INTEGER PRIMARY KEY  AUTOINCREMENT  NOT NULL , `property_id` INTEGER NOT NULL , `item_id` INTEGER, `value` VARCHAR)"
                                ))){
            qDebug() << "SQL Error: " << query->lastError();
            if(logger) logger->writeLog(Logger::Error, Logger::Other, query->lastError().text());
            return;
        }
    }
}

void MainWindow::showReferences()
{
    if(!this->checkUserRights(3)) return;
    if(!checkLicense()) return;
    if(!references_form){
        references_form = new ReferencesForm(ui->mdiArea);
        //references_form->setLogger(logger);
        connect(references_form, SIGNAL(searchItem(int,QString)), this, SLOT(searchItem(int,QString)));
        QMdiSubWindow *subWindowReferences = new QMdiSubWindow;
        subWindowReferences->setWindowIcon(QIcon(":/Icons/icons/Search.png"));
        subWindowReferences->setWidget(references_form);
        ui->mdiArea->addSubWindow(subWindowReferences);
        subWindowReferences->showMaximized();
        if(logger) logger->writeLog(Logger::View, Logger::References);
    }
    else{
        references_form->showMaximized();
    }
}

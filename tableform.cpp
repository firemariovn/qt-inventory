
#include "tableform.h"
#include "ui_tableform.h"
#include "users.h"
#include "items.h"
#include "allocations.h"
#include "headerview.h"
#include "logger.h"

#include <QtSql>
#include <QtGui>
//#include <QLineEdit>

TableForm::TableForm(QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f),
    ui(new Ui::TableForm)
{
    ui->setupUi(this);

    model = 0;
    detailed_model = 0;
    current_row = 0;
    current_id = 0;
    last_filter_field = QString::null;
    last_filter = QString::null;

    show_items = 0;
    show_attachments = 0;
    show_rights = 0;
    show_properties = 0;

    logger = 0;

    headerView = new HeaderView(Qt::Horizontal);
    detailedHeaderView = new HeaderView(Qt::Horizontal);

    ui->tableView->setHorizontalHeader(headerView);
    //ui->tableView->horizontalHeader()->setMovable(true);

    ui->detailed_tableView->hide();
    ui->detailed_tableView->setHorizontalHeader(detailedHeaderView);

    ui->save_toolButton->setEnabled(false);
    ui->revert_toolButton->setEnabled(false);
    ui->attachments_toolButton->setEnabled(false);
    ui->remove_toolButton->setEnabled(false);

    for(int n=0; n<ui->buttonsLayout->count(); n++){
        QWidget* w = ui->buttonsLayout->itemAt(n)->widget();
        if(w){
            w->setStatusTip(w->toolTip());
        }
    }


    connect(this, SIGNAL(showId(bool)), headerView, SLOT(showId(bool)));
    connect(headerView, SIGNAL(updateFields()), this, SLOT(updateFields()));

    connect(ui->save_toolButton, SIGNAL(clicked()), this, SLOT(save()));
    connect(ui->revert_toolButton, SIGNAL(clicked()), this, SLOT(revert()));
    connect(ui->insert_toolButton, SIGNAL(clicked()), this, SLOT(insert()));
    connect(ui->remove_toolButton, SIGNAL(clicked()), this, SLOT(remove()));
    connect(ui->print_toolButton, SIGNAL(clicked()), ui->tableView, SLOT(printPreviewTable()));
    connect(ui->refresh_toolButton, SIGNAL(clicked()), this, SLOT(refresh()));

    connect(ui->filter_comboBox, SIGNAL(textChanged(QString)), this, SLOT(setFilter(QString)));
    connect(ui->field_comboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(setFilterCompleter(QString)));
    connect(ui->clearfilter_toolButton, SIGNAL(clicked()), this, SLOT(clearFilter()));

    connect(ui->attachments_toolButton, SIGNAL(clicked()), this, SLOT(addAttachedFile()));
    connect(ui->detailed_tableView, SIGNAL(add_attachment_()), this, SLOT(addAttachedFile()));
    connect(ui->detailed_tableView, SIGNAL(add_item_()), this, SLOT(addItem()));
    connect(ui->detailed_tableView, SIGNAL(add_property_()), this, SLOT(addProperty()));
    connect(ui->detailed_tableView, SIGNAL(tableUpdate(QString)), this, SLOT(needUpdate(QString)));
    connect(ui->detailed_tableView, SIGNAL(searchItem(int,QString)), this, SIGNAL(searchItem(int,QString)));
    connect(ui->detailed_tableView, SIGNAL(activatedItem(int,QString)), this, SLOT(activatedItem(int,QString)));

    connect(ui->export_toolButton, SIGNAL(clicked()), this, SLOT(exportToCsv()));

    connect(headerView, SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)), this, SLOT(sortIndicatorChanged(int,Qt::SortOrder)));
    connect(ui->detailed_tableView, SIGNAL(tableUpdate(QString)), this, SIGNAL(tableUpdate(QString)));
}

TableForm::~TableForm()
{
    if(headerView) headerView->saveStateToSettings(model->tableName());

    if(model) delete(model);
}

void TableForm::setTable(const QString& table)
{
    if(model) delete(model);
    model = new QSqlRelationalTableModel(this);

    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->setTable(table);
    if(table == "users"){
        this->setWindowTitle(tr("Users"));
        this->setUserRights(this->checkUserRights(1, false));

        model->setHeaderData(model->fieldIndex("username"), Qt::Horizontal, tr("User"), Qt::EditRole);
        model->setHeaderData(model->fieldIndex("firstname"), Qt::Horizontal, tr("First name"), Qt::EditRole);
        model->setHeaderData(model->fieldIndex("lastname"), Qt::Horizontal, tr("Last name"), Qt::EditRole);
        model->setHeaderData(model->fieldIndex("email"), Qt::Horizontal, tr("E-mail"), Qt::EditRole);
    }
    else if(table == "items"){
        this->setWindowTitle(tr("Items"));

        model->setRelation(model->fieldIndex("location_id"), QSqlRelation("locations", "id", "location"));
        model->setRelation(model->fieldIndex("type_id"), QSqlRelation("item_types", "id", "type"));
        model->setRelation(model->fieldIndex("operator_id"), QSqlRelation("operators", "id", "name"));
        model->setRelation(model->fieldIndex("status_id"), QSqlRelation("item_status", "id", "status"));

        this->setUserRights(this->checkUserRights(4, false));
    }
    else if(table == "locations"){
        this->setWindowTitle(tr("Locations"));

        model->setHeaderData(model->fieldIndex("location"), Qt::Horizontal, tr("Location"), Qt::EditRole);
        model->setHeaderData(model->fieldIndex("note"), Qt::Horizontal, tr("Note"), Qt::EditRole);

        this->setUserRights(this->checkUserRights(10, false));
    }
    else if(table == "operators"){
        this->setWindowTitle(tr("Operators"));

        model->setRelation(model->fieldIndex("department_id"), QSqlRelation("departments", "id", "department"));

        this->setUserRights(this->checkUserRights(7, false));
    }
    else if(table == "item_status"){
        this->setWindowTitle(tr("Items status"));

        model->setHeaderData(model->fieldIndex("status"), Qt::Horizontal, tr("Status"), Qt::EditRole);

        this->setUserRights(this->checkUserRights(22, false));
    }
    else if(table == "item_types"){
        this->setWindowTitle(tr("Items types"));

        model->setHeaderData(model->fieldIndex("type"), Qt::Horizontal, tr("Type"), Qt::EditRole);

        this->setUserRights(this->checkUserRights(19, false));
    }
    else if(table == "departments"){
        this->setWindowTitle(tr("Departments"));

        model->setHeaderData(model->fieldIndex("department"), Qt::Horizontal, tr("Department"), Qt::EditRole);

        this->setUserRights(this->checkUserRights(13, false));
    }
    else if(table == "allocations"){
        this->setWindowTitle(tr("Allocations"));
        this->setUserRights(this->checkUserRights(26, false));
        model->setRelation(model->fieldIndex("item_id"), QSqlRelation("items", "id", "number"));
        model->setRelation(model->fieldIndex("to_location_id"), QSqlRelation("locations", "id", "location"));
        model->setRelation(model->fieldIndex("to_operator_id"), QSqlRelation("operators", "id", "name"));
        QSqlQuery* query = new QSqlQuery(QSqlDatabase::database());
        QString temp_locations_table = QUuid::createUuid().toString();
        QString temp_operators_table = QUuid::createUuid().toString();
        if(!query->exec(QString("DROP TABLE IF EXISTS `%1`").arg(temp_locations_table))){
            if(logger) logger->writeLog(Logger::Error, Logger::Allocations,
                                        tr("Sql error:\n%1").arg(query->lastError().text())
                                        );
        }
        if(!query->exec(QString("CREATE TEMP TABLE `%1` (id INTEGER PRIMARY KEY, old_location TEXT)").arg(temp_locations_table))){
            if(logger) logger->writeLog(Logger::Error, Logger::Allocations,
                                        tr("Sql error:\n%1").arg(query->lastError().text())
                                        );
        }
        if(!query->exec(QString("INSERT INTO `%1` ( `id`, `old_location` ) SELECT `id`, `location` FROM locations;").arg(temp_locations_table))){
            if(logger) logger->writeLog(Logger::Error, Logger::Allocations,
                                        tr("Sql error:\n%1").arg(query->lastError().text())
                                        );
            model->setRelation(model->fieldIndex("from_location_id"), QSqlRelation("locations", "id", "location"));
        }
        else{
            model->setRelation(model->fieldIndex("from_location_id"), QSqlRelation(QString("`%1`").arg(temp_locations_table), "id", "old_location"));
        }
        if(!query->exec(QString("DROP TABLE IF EXISTS `%1`").arg(temp_operators_table))){
            if(logger) logger->writeLog(Logger::Error, Logger::Allocations,
                                        tr("Sql error:\n%1").arg(query->lastError().text())
                                        );
        }
        if(!query->exec(QString("CREATE TEMP TABLE `%1` (id INTEGER PRIMARY KEY, old_name TEXT, department_id NUMERIC, note TEXT)").arg(temp_operators_table))){
            if(logger) logger->writeLog(Logger::Error, Logger::Allocations,
                                        tr("Sql error:\n%1").arg(query->lastError().text())
                                        );
        }
        if(!query->exec(QString("INSERT INTO `%1` ( `id`, `old_name`, `department_id`, `note` ) SELECT `id`, `name`, `department_id`, `note` FROM operators;").arg(temp_operators_table))){
            if(logger) logger->writeLog(Logger::Error, Logger::Allocations,
                                        tr("Sql error:\n%1").arg(query->lastError().text())
                                        );
            model->setRelation(model->fieldIndex("from_operator_id"), QSqlRelation("operators", "id", "name"));
        }
        else{
            model->setRelation(model->fieldIndex("from_operator_id"), QSqlRelation(QString("`%1`").arg(temp_operators_table), "id", "old_name"));
        }
    }
    else if(table == "logs"){
        this->setWindowTitle(tr("Logs"));

        model->setRelation(model->fieldIndex("user_id"), QSqlRelation("users", "id", "username"));
    }
    else if(table == "scrap"){
        this->setWindowTitle(tr("Scrap"));

        this->setUserRights(this->checkUserRights(10, false));
    }
    else this->setWindowTitle(model->tableName());
    model->select();

    connect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(dataChanged(QModelIndex, QModelIndex)));

    ui->tableView->disconnect();
    ui->tableView->setModel(model);
    if(!qApp->property("show_id").toBool()) ui->tableView->hideColumn(model->fieldIndex("id"));

    if(model->tableName() == "users"){
        ui->tableView->hideColumn(model->fieldIndex("password"));
        ui->tableView->hideColumn(model->fieldIndex("deleted"));
        ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    }
    else if(model->tableName() == "allocations"){
        ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
        ui->remove_toolButton->hide();

        model->setHeaderData(model->fieldIndex("date"), Qt::Horizontal, tr("Date"), Qt::EditRole);
        model->setHeaderData(model->fieldIndex("number"), Qt::Horizontal, tr("Number"), Qt::EditRole);
        model->setHeaderData(model->fieldIndex("old_location"), Qt::Horizontal, tr("Old location"), Qt::EditRole);
        model->setHeaderData(model->fieldIndex("location"), Qt::Horizontal, tr("Location"), Qt::EditRole);
        model->setHeaderData(model->fieldIndex("old_name"), Qt::Horizontal, tr("Old name"), Qt::EditRole);
        model->setHeaderData(model->fieldIndex("name"), Qt::Horizontal, tr("Name"), Qt::EditRole);
        model->setHeaderData(model->fieldIndex("note"), Qt::Horizontal, tr("Note"), Qt::EditRole);
    }
    else if(model->tableName() == "items"){
        ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
        model->setHeaderData(model->fieldIndex("type"), Qt::Horizontal, tr("Type"), Qt::EditRole);
        model->setHeaderData(model->fieldIndex("status"), Qt::Horizontal, tr("Status"), Qt::EditRole);
        model->setHeaderData(model->fieldIndex("location"), Qt::Horizontal, tr("Location"), Qt::EditRole);
        model->setHeaderData(model->fieldIndex("IP"), Qt::Horizontal, tr("IP address"), Qt::EditRole);
        model->setHeaderData(model->fieldIndex("note"), Qt::Horizontal, tr("Note"), Qt::EditRole);
        model->setHeaderData(model->fieldIndex("model"), Qt::Horizontal, tr("Model"), Qt::EditRole);
        model->setHeaderData(model->fieldIndex("manufacturer"), Qt::Horizontal, tr("Manufacturer"), Qt::EditRole);
        model->setHeaderData(model->fieldIndex("serialno"), Qt::Horizontal, tr("Serial No"), Qt::EditRole);
        model->setHeaderData(model->fieldIndex("number"), Qt::Horizontal, tr("Number"), Qt::EditRole);
        model->setHeaderData(model->fieldIndex("name"), Qt::Horizontal, tr("Name"), Qt::EditRole);
    }
    else if(model->tableName() == "operators"){
        model->setHeaderData(model->fieldIndex("name"), Qt::Horizontal, tr("Name"), Qt::EditRole);
        model->setHeaderData(model->fieldIndex("note"), Qt::Horizontal, tr("Note"), Qt::EditRole);
        model->setHeaderData(model->fieldIndex("department"), Qt::Horizontal, tr("Department"), Qt::EditRole);
    }
    else if(model->tableName() == "logs"){
        ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
        ui->tableView->setSelectionMode(QAbstractItemView::ContiguousSelection);
        ui->insert_toolButton->hide();

        model->setHeaderData(model->fieldIndex("date"), Qt::Horizontal, tr("Date"), Qt::EditRole);
        model->setHeaderData(model->fieldIndex("username"), Qt::Horizontal, tr("Username"), Qt::EditRole);
        model->setHeaderData(model->fieldIndex("table"), Qt::Horizontal, tr("Table"), Qt::EditRole);
        model->setHeaderData(model->fieldIndex("action"), Qt::Horizontal, tr("Action"), Qt::EditRole);
        model->setHeaderData(model->fieldIndex("details"), Qt::Horizontal, tr("Details"), Qt::EditRole);
    }
    else if(model->tableName() == "scrap"){
        ui->remove_toolButton->hide();
        ui->insert_toolButton->hide();

        ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

        ui->tableView->hideColumn(model->fieldIndex("item_id"));
        ui->tableView->hideColumn(model->fieldIndex("allocations_data"));

        model->setHeaderData(model->fieldIndex("type"), Qt::Horizontal, tr("Type"), Qt::EditRole);
        model->setHeaderData(model->fieldIndex("location"), Qt::Horizontal, tr("Location"), Qt::EditRole);
        model->setHeaderData(model->fieldIndex("IP"), Qt::Horizontal, tr("IP address"), Qt::EditRole);
        model->setHeaderData(model->fieldIndex("note"), Qt::Horizontal, tr("Note"), Qt::EditRole);
        model->setHeaderData(model->fieldIndex("model"), Qt::Horizontal, tr("Model"), Qt::EditRole);
        model->setHeaderData(model->fieldIndex("manufacturer"), Qt::Horizontal, tr("Manufacturer"), Qt::EditRole);
        model->setHeaderData(model->fieldIndex("serialno"), Qt::Horizontal, tr("Serial No"), Qt::EditRole);
        model->setHeaderData(model->fieldIndex("number"), Qt::Horizontal, tr("Number"), Qt::EditRole);
        model->setHeaderData(model->fieldIndex("operator"), Qt::Horizontal, tr("Operator"), Qt::EditRole);
        model->setHeaderData(model->fieldIndex("reason"), Qt::Horizontal, tr("Reason"), Qt::EditRole);
        //model->setHeaderData(model->fieldIndex("allocations_data"), Qt::Horizontal, tr("Allocations data"), Qt::EditRole);
    }
    //ui->tableView->setItemDelegate(new QSqlRelationalDelegate(ui->tableView));

    connect(ui->tableView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(itemSelected(QModelIndex,QModelIndex)));
    connect(ui->tableView, SIGNAL(activated(QModelIndex)), ui->tableView, SLOT(activated(QModelIndex)));
    connect(ui->tableView, SIGNAL(activatedItem(int,QString)), this, SLOT(activatedItem(int,QString)));
    connect(ui->tableView, SIGNAL(searchItem(int,QString)), this, SIGNAL(searchItem(int,QString)));

    connect(ui->tableView, SIGNAL(add_item_()), this, SLOT(addItem()));
    connect(ui->tableView, SIGNAL(add_attachment_()), this, SLOT(addAttachedFile()));
    //connect(ui->tableView, SIGNAL(tableUpdate(QString)), this, SLOT(needUpdate(QString)));
    connect(ui->tableView, SIGNAL(tableUpdate(QString)), this, SIGNAL(tableUpdate(QString)));

    if(!headerView->isRestoredState())
        headerView->setRestoredState(headerView->restoreStateFromSettings(model->tableName()));
    if(qApp->property("marking_rows").toBool()){
        ui->tableView->loadMarked();
    }
    else{
        ui->tableView->cleanMarked();
    }
    this->setFilter("");
    this->fillFilterFields();
    this->setDetailedOptions();
    this->restoreFilterState();
}

void TableForm::save()
{
    if(model){
        if(model->tableName() == "users"){
            if(!this->checkUserRights(1)) return;
        }
        else if(model->tableName() == "items"){
            if(!this->checkUserRights(6)) return;
        }
        else if(model->tableName() == "locations"){
            if(!this->checkUserRights(9)) return;
        }
        if(!model->submitAll()){
            if(logger) logger->writeLog(Logger::Error, Logger::Other,
                                        tr("Sql error:\n%1").arg(model->lastError().text())
                                        );
            //qDebug() << "Sql Error: " << model->lastError().text();
            //qDebug() << "Last Sql query: " << model->query().lastQuery();
            return;
        }
        ui->save_toolButton->setEnabled(false);
        ui->revert_toolButton->setEnabled(false);
        emit tableUpdate(model->tableName());
    }
}

void TableForm::revert()
{
    if(model){
        model->revertAll();
        ui->save_toolButton->setEnabled(false);
        ui->revert_toolButton->setEnabled(false);
    }
}

void TableForm::insert()
{
    if(model){
        if(model->tableName() == "users"){
            if(!this->checkUserRights(1)) return;
            Users* dialog = new Users(this);
            dialog->setLogger(logger);
            if(dialog->exec() == QDialog::Accepted){
                emit tableUpdate("Users");
            }
            delete dialog;
        }
        else if(model->tableName() == "items"){
            if(!this->checkUserRights(4)) return;
            addItem();
        }
        else if(model->tableName() == "item_types"){
            if(!this->checkUserRights(19)) return;
            bool ok;
            QString text = QInputDialog::getText(this, tr("Add item type"),
                                                      tr("Item type:"), QLineEdit::Normal,
                                                      "", &ok);
            if (ok && !text.isEmpty()){
                QSqlQuery* query = new QSqlQuery();
                if(!query->exec(QString("INSERT INTO item_types (`type`) VALUES ('%1')")
                                .arg(text))){
                    if(logger) logger->writeLog(Logger::Error, Logger::Item_types,
                                            tr("Can not write into database. Sql error:\n%1")
                                            .arg(query->lastError().text())
                                            );
                }
                else{
                    if(logger) logger->writeLog(Logger::Add, Logger::Item_types, text);
                    emit tableUpdate("item_types");
                }
            }
        }
        else if(model->tableName() == "item_status"){
            if(!this->checkUserRights(22)) return;

            bool ok;
            QString text = QInputDialog::getText(this, tr("Add item status"),
                                                      tr("Item status:"), QLineEdit::Normal,
                                                      "", &ok);
            if (ok && !text.isEmpty()){
                QSqlQuery* query = new QSqlQuery();
                if(!query->exec(QString("INSERT INTO item_status (`status`) VALUES ('%1')")
                                .arg(text))){
                    if(logger) logger->writeLog(Logger::Error, Logger::Item_status,
                                            tr("Can not write into database. Sql error:\n%1")
                                            .arg(query->lastError().text())
                                            );
                }
                else{
                    if(logger) logger->writeLog(Logger::Add, Logger::Item_status, text);
                    emit tableUpdate("item_status");
                }
            }
        }

        else if(model->tableName() == "locations"){
            if(!this->checkUserRights(10)) return;
            bool ok;
            QString text = QInputDialog::getText(this, tr("Add location"),
                                                      tr("Location:"), QLineEdit::Normal,
                                                      "", &ok);
            if (ok && !text.isEmpty()){
                QSqlQuery* query = new QSqlQuery();
                if(!query->exec(QString("INSERT INTO locations (`location`) VALUES ('%1')")
                                .arg(text))){
                    if(logger) logger->writeLog(Logger::Error, Logger::Locations,
                                            tr("Can not write into database. Sql error:\n%1")
                                            .arg(query->lastError().text())
                                            );
                }
                else{
                    if(logger) logger->writeLog(Logger::Add, Logger::Locations, text);
                    emit tableUpdate("locations");
                }
            }
        }

        else if(model->tableName() == "operators"){
            if(!this->checkUserRights(7)) return;
            bool ok;
            QString text = QInputDialog::getText(this, tr("Add operator"),
                                                      tr("Operator:"), QLineEdit::Normal,
                                                      "", &ok);
            if (ok && !text.isEmpty()){
                QSqlQuery* query = new QSqlQuery();
                if(!query->exec(QString("INSERT INTO operators (`name`,`department_id`) VALUES ('%1', '%2')")
                                .arg(text)
                                .arg(nullDepartmentId()))
                        ){
                    if(logger) logger->writeLog(Logger::Error, Logger::Operators,
                                            tr("Can not write into database. Sql error:\n%1")
                                            .arg(query->lastError().text())
                                            );
                }
                else{
                    if(logger) logger->writeLog(Logger::Add, Logger::Operators, text);
                    emit tableUpdate("operators");
                }
            }
        }

        else if(model->tableName() == "departments"){
            if(!this->checkUserRights(13)) return;
            bool ok;
            QString text = QInputDialog::getText(this, tr("Add department"),
                                                      tr("Department:"), QLineEdit::Normal,
                                                      "", &ok);
            if (ok && !text.isEmpty()){
                QSqlQuery* query = new QSqlQuery();
                if(!query->exec(QString("INSERT INTO departments (`department`) VALUES ('%1')")
                                .arg(text))){
                    if(logger) logger->writeLog(Logger::Error, Logger::Departments,
                                            tr("Can not write into database. Sql error:\n%1")
                                            .arg(query->lastError().text())
                                            );
                }
                else{
                    if(logger) logger->writeLog(Logger::Add, Logger::Departments, text);
                    emit tableUpdate("departments");
                }
            }
        }
        else if(model->tableName() == "allocations"){
            if(!this->checkUserRights(26)) return;
            Allocations* dialog = new Allocations(this);
            dialog->setWindowFlags( dialog->windowFlags()| Qt::WindowMaximizeButtonHint);
            dialog->setLogger(logger);
            if(dialog->exec() == QDialog::Accepted){
                emit tableUpdate("Allocations");
            }
            delete dialog;
        }
    }
}

void TableForm::remove()
{
    if(!model) return;
    QModelIndex index = ui->tableView->selectionModel()->currentIndex();
    if(index.isValid()){
        if(model->tableName() == "users"){
            this->removeUser(index);
        }

        else if(model->tableName() == "items"){
            this->removeItem(index);
        }

        else if(model->tableName() == "locations"){
            this->removeLocation(index);
        }

        else if(model->tableName() == "item_types"){
            this->removeItemType(index);
        }

        else if(model->tableName() == "item_status"){
            this->removeItemStatus(index);
        }

        else if(model->tableName() == "operators"){
            this->removeOperator(index);
        }

        else if(model->tableName() == "departments"){
            this->removeDepartments(index);
        }

        else if(model->tableName() == "logs"){
            this->removeLogs(ui->tableView->selectionModel()->selectedRows(model->fieldIndex("id")));
        }
    }
}

void TableForm::dataChanged(const QModelIndex & topLeft, const QModelIndex & bottomRigh)
{
    ui->save_toolButton->setEnabled(true);
    ui->revert_toolButton->setEnabled(true);
}

void TableForm::closeEvent(QCloseEvent *event)
 {
    if(ui->save_toolButton->isEnabled()){
        //qDebug() << "There are unsaved changes";
        int ret = QMessageBox::warning(this, tr("Inventory"),
                                        tr("There are unsaved changes.\n"
                                           "Do you want to save?"),
                                        QMessageBox::Save | QMessageBox::Discard
                                        | QMessageBox::Cancel,
                                        QMessageBox::Save);
        switch (ret) {
           case QMessageBox::Save:
           this->save();
               break;
           case QMessageBox::Discard:
           this->revert();
               break;
           case QMessageBox::Cancel:
               event->ignore();
               return;
           default:
               // should never be reached
               break;
        }
    }
    headerView->saveStateToSettings(model->tableName());
    event->accept();
 }

void TableForm::fillFilterFields()
{
    QSqlRecord record = model->record();
    if(record.isEmpty()){
        return;
    }
    int current_index = ui->field_comboBox->currentIndex();
    ui->field_comboBox->clear();
    ui->field_comboBox->addItem(tr("All"));
    for(int i=0; i < record.count(); i++){
        if(!ui->tableView->isColumnHidden(i))
            ui->field_comboBox->addItem(transFilterField(record.fieldName(i)));
    }
    if(current_index > -1) ui->field_comboBox->setCurrentIndex(current_index);
    this->setFilter(ui->filter_comboBox->currentText());
}

void TableForm::setFilter(const QString& filter)
{
    QStringList tableWithField_deleted;
    tableWithField_deleted << "users";
    bool deleted = tableWithField_deleted.contains(model->tableName());
    if(filter.isEmpty()){
        if(deleted){
            model->setFilter(QString("%1.deleted=%2")
                .arg(model->tableName())
                .arg(0)
                         );
        }
        else model->setFilter(filter);
    }
    else {
        if(ui->field_comboBox->currentText() == tr("All")){
            QString fields;
            for(int n=1; n<ui->field_comboBox->count(); n++){
                QString field;
                if(model->relation(model->fieldIndex(transFilterField(ui->field_comboBox->itemText(n), false))).isValid()){
                    field = transFilterField(ui->field_comboBox->itemText(n), false);
                }
                else{
                    field = QString("%1.`%2`")
                            .arg(model->tableName())
                            .arg(transFilterField(ui->field_comboBox->itemText(n), false));
                }
                if(deleted){
                    fields.append(QString("%1 LIKE '%%4%' AND %2.deleted=%3")
                                  .arg(field)
                                  .arg(model->tableName())
                                  .arg(0)
                                  .arg(filter));
                }
                else{
                    fields.append(QString("%1 LIKE '%%2%'")
                                  .arg(field)
                                  .arg(filter));
                }
                if(n < ui->field_comboBox->count()-1) fields.append(" OR ");
                }
                model->setFilter(fields);
            }
        else{
            QString field;
            if(model->relation(model->fieldIndex(transFilterField(ui->field_comboBox->currentText(), false))).isValid()){
                field = transFilterField(ui->field_comboBox->currentText(), false);
            }
            else{
                field = QString("%1.`%2`")
                        .arg(model->tableName())
                        .arg(transFilterField(ui->field_comboBox->currentText(), false));
            }
            if(deleted){
                model->setFilter(QString("%1 LIKE '%%4%' AND %2.deleted=%3")
                             .arg(field)
                             .arg(model->tableName())
                             .arg(0)
                             .arg(filter)
                             );
            }
            else{
                model->setFilter(QString("%1 LIKE '%%2%'")
                             .arg(field)
                             .arg(filter)
                             );
            }
        }
    }
    ui->tableView->restoreMarked();
    if(model && ui->detailed_toolButton->isChecked())
            this->updateDetailedView();
    //qDebug() << "Last model filter: " << model->filter();
    //qDebug() << "Last sql error: " << model->lastError().text();
}

void TableForm::setFilterCompleter(const QString& field)
{
    if(!model) return;
    QSqlRelationalTableModel* c_model = new QSqlRelationalTableModel(this);
    c_model->setTable(model->tableName());
    for(int i=0; i<model->columnCount(); i++){
        if(model->relation(i).isValid()) c_model->setRelation(i, model->relation(i));
    }
    c_model->select();
    QStringList wordList;
    for(int i=0; i<c_model->rowCount(); i++){
        QSqlRecord record = c_model->record(i);
        if(record.isEmpty()) break;
        wordList << record.value(transFilterField(field, false)).toString();
    }
    wordList.removeDuplicates();
    QCompleter *completer = new QCompleter(wordList, this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    ui->filter_comboBox->clear();
    ui->filter_comboBox->setCompleter(completer);
    ui->filter_comboBox->addItems(wordList);
    delete c_model;
}

bool TableForm::checkUserRights(const int idx, const bool show_message) const
{
    QBitArray rights(qApp->property("user_rights").toBitArray());
    if(!rights[idx]){
        if(show_message){
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

void TableForm::setUserRights(const bool has_rights)
{
    if(!has_rights){
        ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
        ui->insert_toolButton->setEnabled(false);
        ui->remove_toolButton->setEnabled(false);
        this->setWindowTitle(this->windowTitle().append(tr(" (read only)")));
    }
}

void TableForm::itemSelected(const QModelIndex &current, const QModelIndex& previous)
{
    /*
    if(!query->exec(QString("DROP TABLE IF EXISTS `ITEM_PROPERTIES_TEMP`"))){
        if(logger) logger->writeLog(Logger::Error, Logger::Other,
                                    tr("Sql error:\n%1").arg(query->lastError().text())
                                    );
    }
    */
    if(current.isValid()){
        setCurrentIndex(current);
        ui->remove_toolButton->setEnabled(true);
        ui->attachments_toolButton->setEnabled(true);
        current_row = current.row();
        if(model){
            QSqlRecord record = model->record(current_row);
            if(!record.isEmpty()){
                current_id = record.value("id").toInt();
                /*
                if(model->tableName() == "items"){
                    qDebug() << "Create temporary properties table";
                    if(!query->exec(QString("CREATE TEMP TABLE `ITEM_PROPERTIES_TEMP` (id INTEGER PRIMARY KEY, property VARCHAR, value VARCHAR)"))){
                        if(logger) logger->writeLog(Logger::Error, Logger::Other,
                                                    tr("Sql error:\n%1").arg(query->lastError().text())
                                                    );
                    }
                    if(!query->exec(QString("INSERT INTO `%1` ( `id`, `property_id`, `value` ) "
                                            "SELECT item_properties.id, item_properties.property_id, item_properties.value "
                                            "FROM item_properties "
                                            "LEFT JOIN properties "
                                            "ON data.type_id = properties.type_id "
                                            "LEFT JOIN properties_data "
                                            "ON properties.id = properties_data.property_id AND properties_data.data_id = data.id").arg(temp_locations_table))){
                        if(logger) logger->writeLog(Logger::Error, Logger::Allocations,
                                                    tr("Sql error:\n%1").arg(query->lastError().text())
                                                    );
                }
                */
            }
            else{
                current_id = 0;
            }
            if( ui->detailed_toolButton->isChecked()) this->updateDetailedView();
        }
        //qDebug() << QString("Set Current index row: %1, column: %2, data: %3").arg(current.row())
          //          .arg(current.column()).arg(current.data().toString());
    }
    else{
        ui->remove_toolButton->setEnabled(false);
        ui->attachments_toolButton->setEnabled(false);
        current_row = 0;
        current_id = 0;
    }
}

void TableForm::addAttachedFile()
{
    if(!this->checkUserRights(16)) return;
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), qApp->property("last_attached_dir").toString());
    if(fileName.isEmpty()) return;
    //qDebug() << "Choosen file: " << fileName;
    /* 1 - copy file to attacments folder */
    QFileInfo fi(qApp->property("data_path").toString());
    QDir dir(fi.absoluteDir());
    if(!dir.exists()){
        if(logger) logger->writeLog(Logger::Error, Logger::Attachments,
                                    tr("Can not find data directory!"));
        return;
    }
    if(!dir.cd("attachments")){
        if(logger) logger->writeLog(Logger::Error, Logger::Attachments,
                                    tr("Attachments directory does not exist.\nTry to create it"));
        if(!dir.mkdir("attachments")){
            if(logger) logger->writeLog(Logger::Error, Logger::Attachments,
                                        tr("Error! Can not create attachments directory. The operation is interrupted"));
            return;
        }
        if(!dir.cd("attachments")){
            if(logger) logger->writeLog(Logger::Error, Logger::Attachments,
                                        tr("Attachments directory does not exist. The operation is interrupted"));
            return;
        }
    }
    QFile source(fileName);
    fi.setFile(source);
    qApp->setProperty("last_attached_dir", fi.absolutePath());
    this->setCursor(Qt::BusyCursor);
    if(!source.copy(dir.absoluteFilePath(fi.fileName()))){
        this->setCursor(Qt::ArrowCursor);
        if(dir.exists(fi.fileName())) {
            if(logger) logger->writeLog(Logger::Error, Logger::Attachments,
                                        tr("File already exists!"));
        }
        else{
            if(logger) logger->writeLog(Logger::Error, Logger::Attachments,
                                        tr("Can not copy file!"));
            return;
        }
    }
    else{
        this->setCursor(Qt::ArrowCursor);
        if(logger) logger->writeLog(Logger::Add, Logger::Attachments,
                                    tr("File was successfully copied:\n%1").arg(dir.absoluteFilePath(fi.fileName()))
                                    );
        //qDebug() << "File was successfully copied :)";
    }
    if(model){
        QSqlRecord record = model->record(current_row);
        if(!record.isEmpty()){
            QSqlQuery* query = new QSqlQuery(QSqlDatabase::database());
            if(!query->exec(QString("INSERT INTO attachments (`item_id`, `tablename`, `filename`) VALUES ('%1', '%2', '%3')")
                            .arg(record.value("id").toInt())
                            .arg(model->tableName())
                            .arg(fi.fileName()))
               ){
                if(logger) logger->writeLog(Logger::Error, Logger::Attachments,
                                        tr("Sql error:\n%1").arg(query->lastError().text())
                                        );
            }
            else{
                if(logger) logger->writeLog(Logger::Add, Logger::Attachments,
                                            tr("Attacment: %1 has been added to:\n%2")
                                            .arg(fileName)
                                            .arg(logger->infoLog(record))
                                            );
            }
        }
        this->updateDetailedView();
    }
}

void TableForm::showDetailedTable(const bool show)
{
    if(show){
        ui->detailed_tableView->show();
    }
    else{
        QList<int> currentSizes = ui->splitter->sizes();
        currentSizes[0] += currentSizes[1];
        currentSizes[1] = 0;
        ui->detailed_tableView->hide();
        ui->splitter->setSizes(currentSizes);
    }
    this->updateDetailedView();
}

void TableForm::showDetailedItems(const bool show)
{
    if(show_attachments){
        show_attachments->blockSignals(true);
        show_attachments->setChecked(false);
        show_attachments->blockSignals(false);
    }
    if(show_rights){
        show_rights->blockSignals(true);
        show_rights->setChecked(false);
        show_rights->blockSignals(false);
    }
    if(show_properties){
        show_properties->blockSignals(true);
        show_properties->setChecked(false);
        show_properties->blockSignals(false);
    }
    if(!show){
        if(ui->detailed_toolButton->isChecked()) ui->detailed_toolButton->toggle();
        return;
    }
    if(!this->checkUserRights(3)) return;

    if(!ui->detailed_toolButton->isChecked()) ui->detailed_toolButton->toggle();
    if(model->tableName() == "operators"){
        detailed_filter = QString("operator_id = ");
    }
    else if(model->tableName() == "locations"){
        detailed_filter = QString("location_id = ");
    }
    else if(model->tableName() == "item_types"){
        detailed_filter = QString("type_id = ");
    }
    else if(model->tableName() == "item_status"){
        detailed_filter = QString("status_id = ");
    }
    //if(!detailed_model) detailed_model = new QSqlRelationalTableModel;
    if(!detailed_model) detailed_model = new MySqlRelationalTableModel(ui->detailed_tableView);
    detailed_model->clear();
    detailed_model->setTable("items");
    detailed_model->setRelation(detailed_model->fieldIndex("type_id"), QSqlRelation("item_types", "id", "type"));
    detailed_model->setRelation(detailed_model->fieldIndex("operator_id"), QSqlRelation("operators", "id", "name"));
    detailed_model->setRelation(detailed_model->fieldIndex("status_id"), QSqlRelation("item_status", "id", "status"));
    detailed_model->setRelation(detailed_model->fieldIndex("location_id"), QSqlRelation("locations", "id", "location"));
    detailed_model->select();
    ui->detailed_tableView->setModel(detailed_model);
    ui->detailed_tableView->hideColumn(detailed_model->fieldIndex("id"));
    //ui->detailed_tableView->hideColumn(detailed_model->fieldIndex("location_id"));
    //ui->detailed_tableView->hideColumn(detailed_model->fieldIndex("operator_id"));

    //ui->detailed_tableView->setItemDelegate(new QSqlRelationalDelegate(ui->tableView));

    detailed_model->setHeaderData(detailed_model->fieldIndex("type"), Qt::Horizontal, tr("Type"), Qt::EditRole);
    detailed_model->setHeaderData(detailed_model->fieldIndex("status"), Qt::Horizontal, tr("Status"), Qt::EditRole);
    detailed_model->setHeaderData(detailed_model->fieldIndex("location"), Qt::Horizontal, tr("Location"), Qt::EditRole);
    detailed_model->setHeaderData(detailed_model->fieldIndex("IP"), Qt::Horizontal, tr("IP address"), Qt::EditRole);
    detailed_model->setHeaderData(detailed_model->fieldIndex("note"), Qt::Horizontal, tr("Note"), Qt::EditRole);
    detailed_model->setHeaderData(detailed_model->fieldIndex("model"), Qt::Horizontal, tr("Model"), Qt::EditRole);
    detailed_model->setHeaderData(detailed_model->fieldIndex("manufacturer"), Qt::Horizontal, tr("Manufacturer"), Qt::EditRole);
    detailed_model->setHeaderData(detailed_model->fieldIndex("serialno"), Qt::Horizontal, tr("Serial No"), Qt::EditRole);
    detailed_model->setHeaderData(detailed_model->fieldIndex("number"), Qt::Horizontal, tr("Number"), Qt::EditRole);
    detailed_model->setHeaderData(detailed_model->fieldIndex("name"), Qt::Horizontal, tr("Name"), Qt::EditRole);

    this->updateDetailedView();
}

void TableForm::updateDetailedView()
{
    if(!detailed_model) return;
    QString filter = detailed_filter;
    detailed_model->setFilter(filter.append(QString("'%1'").arg(QString::number(current_id))));

    if(qApp->property("marking_rows").toBool()){
        ui->detailed_tableView->loadMarked();
    }
    else
        ui->detailed_tableView->cleanMarked();

}

void TableForm::setDetailedOptions()
{
    ui->detailed_toolButton->hide();
    if(!model) return;
    ui->detailed_toolButton->show();

    QMenu* detailed = new QMenu(ui->detailed_toolButton);

    show_attachments = new QAction(QIcon(":/Icons/icons/Attach.png"), tr("Show attachments"), detailed);
    show_attachments->setCheckable(true);
    show_attachments->setChecked(false);
    show_attachments->setStatusTip(tr("Shows attachments associated with the selected row"));

    detailed->addAction(show_attachments);
    connect(show_attachments, SIGNAL(toggled(bool)), this, SLOT(showDetailedAttachments(bool)));

    connect(ui->detailed_toolButton, SIGNAL(toggled(bool)), this, SLOT(showDetailedTable(bool)));

    if(model->tableName() == "operators" ||
            model->tableName() == "locations" ||
            model->tableName() == "item_types" ||
            model->tableName() == "item_status"){
        show_items = new QAction(QIcon(":/Icons/icons/Computer.png"), tr("Show items"), detailed );
        show_items->setCheckable(true);
        show_items->setChecked(false);
        show_items->setStatusTip(tr("Shows items associated with the selected row"));

        detailed->addAction(show_items);
        connect(show_items, SIGNAL(toggled(bool)), this, SLOT(showDetailedItems(bool)));
    }

    if(model->tableName() == "users"){
        show_rights = new QAction(tr("Show users rights"), detailed );
        show_rights->setCheckable(true);
        show_rights->setChecked(false);

        detailed->addAction(show_rights);
        connect(show_rights, SIGNAL(toggled(bool)), this, SLOT(showDetailedRights(bool)));
    }

    if(model->tableName() == "item_types"){
        show_properties = new QAction(QIcon(":/Icons/icons/Tag.png"), tr("Show properties"), detailed );
        show_properties->setCheckable(true);
        show_properties->setChecked(false);
        show_properties->setStatusTip(tr("Shows properties associated with the selected item type"));
        detailed->addAction(show_properties);
        connect(show_properties, SIGNAL(toggled(bool)), this, SLOT(showTypeProperties(bool)));
    }

    if(model->tableName() == "items"){
        show_properties = new QAction(QIcon(":/Icons/icons/Tag.png"), tr("Show properties"), detailed );
        show_properties->setCheckable(true);
        show_properties->setChecked(false);
        show_properties->setStatusTip(tr("Shows properties of selected item"));
        detailed->addAction(show_properties);
        connect(show_properties, SIGNAL(toggled(bool)), this, SLOT(showItemProperties(bool)));
    }
    ui->detailed_toolButton->setMenu(detailed);
}

void TableForm::showDetailedAttachments(const bool show)
{
    if(show_items){
        show_items->blockSignals(true);
        show_items->setChecked(false);
        show_items->blockSignals(false);
    }
    if(show_rights){
        show_rights->blockSignals(true);
        show_rights->setChecked(false);
        show_rights->blockSignals(false);
    }
    if(show_properties){
        show_properties->blockSignals(true);
        show_properties->setChecked(false);
        show_properties->blockSignals(false);
    }
    if(!show){
        if(ui->detailed_toolButton->isChecked()) ui->detailed_toolButton->toggle();
        return;
    }
    if(!this->checkUserRights(15)) return;

    if(!ui->detailed_toolButton->isChecked()) ui->detailed_toolButton->toggle();
    detailed_filter.clear();
    detailed_filter = QString("`tablename` LIKE '%1' AND `item_id`=")
            .arg(model->tableName());

    if(!detailed_model) detailed_model = new MySqlRelationalTableModel(ui->detailed_tableView);
    detailed_model->clear();
    detailed_model->setTable("attachments");
    detailed_model->select();
    ui->detailed_tableView->setModel(detailed_model);
    ui->detailed_tableView->hideColumn(detailed_model->fieldIndex("id"));
    ui->detailed_tableView->hideColumn(detailed_model->fieldIndex("item_id"));
    ui->detailed_tableView->hideColumn(detailed_model->fieldIndex("tablename"));

    detailed_model->setHeaderData(detailed_model->fieldIndex("filename"), Qt::Horizontal, tr("File name"), Qt::EditRole);

    this->updateDetailedView();
}

void TableForm::needUpdate(const QString &table)
{
    if(model){
        this->saveFilterState();
        QByteArray header_state = ui->tableView->horizontalHeader()->saveState();
        QString _table = model->tableName();
        model->submitAll();
        this->setTable(_table);
        this->restoreFilterState();
        ui->tableView->horizontalHeader()->restoreState(header_state);
        ui->tableView->selectionModel()->setCurrentIndex(currentIndex(),
                                                                QItemSelectionModel::Toggle | QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);
    }
    if(detailed_model && detailed_model->tableName() == table){
        if(qApp->property("marking_rows").toBool()){
            ui->detailed_tableView->loadMarked();
        }
    }
}

void TableForm::showDetailedRights(const bool show)
{
    if(show_items){
        show_items->blockSignals(true);
        show_items->setChecked(false);
        show_items->blockSignals(false);
    }
    if(show_attachments){
        show_attachments->blockSignals(true);
        show_attachments->setChecked(false);
        show_attachments->blockSignals(false);
    }
    if(show_properties){
        show_properties->blockSignals(true);
        show_properties->setChecked(false);
        show_properties->blockSignals(false);
    }
    if(!show) {
        if(ui->detailed_toolButton->isChecked()) ui->detailed_toolButton->toggle();
        return;
    }
    if(!this->checkUserRights(0)) return;

    if(!ui->detailed_toolButton->isChecked()) ui->detailed_toolButton->toggle();
    detailed_filter.clear();
    detailed_filter = QString("`user_id`=");
    if(!detailed_model) detailed_model = new MySqlRelationalTableModel(ui->detailed_tableView);
    detailed_model->clear();
    detailed_model->setTable("users_rights");
    detailed_model->select();
    ui->detailed_tableView->setModel(detailed_model);
    ui->detailed_tableView->hideColumn(detailed_model->fieldIndex("id"));
    ui->detailed_tableView->hideColumn(detailed_model->fieldIndex("user_id"));

    this->updateDetailedView();
}

void TableForm::activatedItem(const int item_id, const QString &table)
{
    if(table == "items") editItem(item_id);
    else if(table == "allocations") editAllocation(item_id);
    else if(table == "users") editUser(item_id);
}

void TableForm::editAllocation(const int id)
{
    if(!this->checkUserRights(25)) return;
    Allocations* allocations_dialog = new Allocations(this);
    allocations_dialog->setLogger(logger);
    allocations_dialog->setEditMode(true);
    allocations_dialog->loadAllocation(id);
    if(allocations_dialog->exec() == QDialog::Accepted){
        emit tableUpdate("Allocations");
    }
    delete allocations_dialog;
}

void TableForm::editUser(const int id)
{
    if(!this->checkUserRights(1)) return;
    Users* users_dialog = new Users(this);
    if(logger) users_dialog->setLogger(logger);
    users_dialog->setEditMode(true);
    users_dialog->loadUser(id);
    if(users_dialog->exec() == QDialog::Accepted){
        emit tableUpdate("Users");
    }
    delete users_dialog;
}

void TableForm::editItem(const int id)
{
    if(!this->checkUserRights(4)) return;
    if(id == nullItemId()) {
        QMessageBox msgBox;
        msgBox.setText(tr("Editing of NULL item is not allowed!"));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        return;
    }
    Items* items_dialog = new Items(this);
    if(logger) items_dialog->setLogger(logger);
    items_dialog->setEditMode(true);
    items_dialog->loadItem(id);
    if(items_dialog->exec() == QDialog::Accepted){
        emit tableUpdate("Items");
    }
    delete items_dialog;
}

void TableForm::addItem()
{
    Items* dialog = new Items(this);
    dialog->setLogger(logger);
    if(model->tableName() == "operators"){
        dialog->setOperatorFilter(current_id);
    }
    else if(model->tableName() == "locations"){
        dialog->setLocationFilter(current_id);
    }
    else if(model->tableName() == "item_types"){
        dialog->setTypeFilter(current_id);
    }
    else if(model->tableName() == "item_status"){
        dialog->setStatusFilter(current_id);
    }
    if(dialog->exec() == QDialog::Accepted){
        emit tableUpdate("Items");
    }
    delete dialog;
}

void TableForm::removeItemType(const QModelIndex &index)
{
    if(!this->checkUserRights(20)) return;
    QSqlRecord record = model->record(index.row());
    if(nullTypeId() == record.value("id").toInt()){
        QMessageBox msgBox;
        msgBox.setText(tr("Deletion of NULL item type is not allowed!"));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        return;
    }
    int ret = QMessageBox::question(this, tr("Item types"),
                                    tr("Do you want to delete type %1?\nThis could not be restored!")
                                    .arg(record.value("type").toString()),
                                    QMessageBox::Ok | QMessageBox::Cancel);
    if(ret == QMessageBox::Cancel) return;
    int old_id = record.value("id").toInt();
    if(!model->removeRow(index.row())){
        if(logger) logger->writeLog(Logger::Error, Logger::Item_types,
                                    tr("Sql error:\n%1").arg(model->lastError().text())
                                    );
        return;
    }
    this->save();
    QSqlQuery* query = new QSqlQuery(QSqlDatabase::database());
    int null_id = nullTypeId();
    if(!query->exec(QString("UPDATE items SET `type_id`=%1 WHERE `type_id`=%2")
                    .arg(null_id)
                    .arg(old_id))){
        if(logger) logger->writeLog(Logger::Error, Logger::Item_types,
                                    tr("Sql error:\n%1").arg(query->lastError().text())
                                    );
    }
    this->save();
}

void TableForm::removeItem(const QModelIndex &index)
{
    if(!this->checkUserRights(7)) return;
    QSqlRecord record = model->record(index.row());
    if(nullItemId() == record.value("id").toInt()){
        QMessageBox msgBox;
        msgBox.setText(tr("Deletion of NULL item is not allowed!"));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        return;
    }
    QSqlQuery* query = new QSqlQuery(QSqlDatabase::database());
    if(!query->exec(QString("SELECT * FROM allocations WHERE `item_id`=%1")
                    .arg(record.value("id").toInt()))){
        if(logger) logger->writeLog(Logger::Error, Logger::Items,
                                    tr("Sql error:\n%1").arg(query->lastError().text())
                                    );
    }
    if(query->first()){
        int ret = QMessageBox::question(this, tr("Items"),
                                        tr("Do you want to delete %1 %2?\nThis could not be restored!\n"
                                           "This item is found in some allocations.\nWill be made automatically reset of the these allocations!")
                                        .arg(record.value("type").toString())
                                        .arg(record.value("number").toString()),
                                        QMessageBox::Ok | QMessageBox::Cancel);
        if(ret == QMessageBox::Cancel) return;
        int null_id = nullItemId();
        QSqlQuery* update_query = new QSqlQuery(QSqlDatabase::database());
        QString note(tr("Revised allocation. Deleted item:\ntype: %1, manufacturer: %2, number: %3\n"
                        "serialNo: %4, IP: %5, note: %6\n,"
                        "status: %7, location: %8,\nname: %9.")
                     .arg(record.value("type").toString())
                     .arg(record.value("manufacturer").toString())
                     .arg(record.value("number").toString())
                     .arg(record.value("serialno").toString())
                     .arg(record.value("IP").toString())
                     .arg(record.value("note").toString())
                     .arg(record.value("status").toString())
                     .arg(record.value("location").toString())
                     .arg(record.value("name").toString())
                     );
        do {
            QSqlRecord r = query->record();
            note.append(tr("\nPrevious allocation date: %1").arg(r.value("date").toString()));
            note.append(tr("\nPrevious allocation note: %1").arg(r.value("note").toString()));

            if(!update_query->exec(QString("UPDATE allocations SET "
                                           "`item_id`=%1,`date`='%2', `note`='%3' WHERE `id`=%4")
                            .arg(null_id)
                            .arg(QDate::currentDate().toString("yyyy-MM-dd"))
                            .arg(note)
                            .arg(r.value("id").toInt())
                            )){
                if(logger) logger->writeLog(Logger::Error, Logger::Allocations,
                                            tr("Sql error:\n%1").arg(query->lastError().text())
                                            );
            }
            else{
                if(logger) logger->writeLog(Logger::Delete, Logger::Allocations,
                                            tr("Update allocation:\n%1")
                                            .arg(logger->infoLog(r)));
            }
        }
        while(query->next());
    }
    else{
        int ret = QMessageBox::question(this, tr("Items"),
                                        tr("Do you want to delete %1 %2?\nThis could not be restored!")
                                        .arg(record.value("type").toString())
                                        .arg(record.value("number").toString()),
                                        QMessageBox::Ok | QMessageBox::Cancel);
        if(ret == QMessageBox::Cancel) return;
    }
    if(!query->exec(QString("DELETE FROM items WHERE `id`=%1")
                    .arg(record.value("id").toInt()))){
        if(logger) logger->writeLog(Logger::Error, Logger::Items,
                                    tr("Sql error:\n%1").arg(query->lastError().text())
                                    );
    }
    if(logger) logger->writeLog(Logger::Delete, Logger::Items,
                                tr("Deleted item:\n%1").arg(logger->infoLog(record)));
    this->save();
}

void TableForm::removeUser(const QModelIndex &index)
{
    if(!this->checkUserRights(2)) return;
    QSqlRecord record = model->record(index.row());
    int ret = QMessageBox::question(this, tr("Users"),
                                tr("Do you want to delete %1?\nThis could not be restored!")
                                .arg(record.value("username").toString()),
                                QMessageBox::Ok | QMessageBox::Cancel);
    if(ret == QMessageBox::Cancel) return;
    QSqlQuery* query = new QSqlQuery(QSqlDatabase::database());
    if(!query->exec(QString("UPDATE users SET `deleted`=1 WHERE `id`=%1")
                    .arg(record.value("id").toInt()))){
        if(logger) logger->writeLog(Logger::Error, Logger::Users,
                                    tr("Sql error:\n%1").arg(query->lastError().text())
                                    );
        return;
    }
    /*else {
        QSqlQuery* query = new QSqlQuery(QSqlDatabase::database());
        if(!query->exec(QString("DELETE FROM `users_rights` WHERE `user_id`=%1")
                            .arg(record.value("id").toInt()))){
            qDebug() << "Can not remove rights data from database. Sql error: " << query->lastError().text();
        }
    }
    */
    if(logger) logger->writeLog(Logger::Delete, Logger::Users, logger->infoLog(record));
    this->save();
}

void TableForm::removeLocation(const QModelIndex &index)
{
    if(!this->checkUserRights(10)) return;
    QSqlRecord record = model->record(index.row());
    if(nullLocationId() == record.value("id").toInt()){
        QMessageBox msgBox;
        msgBox.setText(tr("Deletion of NULL locaton is not allowed!"));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        return;
    }
    int ret = QMessageBox::question(this, tr("Locations"),
                                    tr("Do you want to delete %1?\nThis could not be restored!")
                                    .arg(record.value("location").toString()),
                                    QMessageBox::Ok | QMessageBox::Cancel);
    if(ret == QMessageBox::Cancel) return;
    int old_id = record.value("id").toInt();
    if(!model->removeRow(index.row())){
        if(logger) logger->writeLog(Logger::Error, Logger::Locations,
                                    tr("Sql error:\n%1").arg(model->lastError().text()));
        return;
    }
    if(logger) logger->writeLog(Logger::Delete, Logger::Locations, logger->infoLog(record));
    this->save();
    QSqlQuery* query = new QSqlQuery(QSqlDatabase::database());
    int null_id = nullLocationId();
    if(!query->exec(QString("UPDATE items SET `location_id`=%1 WHERE `location_id`=%2")
                    .arg(null_id)
                    .arg(old_id))){
        if(logger) logger->writeLog(Logger::Error, Logger::Items,
                                    tr("Sql error:\n%1").arg(query->lastError().text())
                                    );
    }
    else{
        if(logger) logger->writeLog(Logger::Edit, Logger::Items,
                                    tr("Set null location id for items with location id: %1").arg(old_id)
                                    );
    }
    if(!query->exec(QString("SELECT * FROM allocations WHERE `from_location_id`=%1")
                    .arg(old_id))){
        if(logger) logger->writeLog(Logger::Error, Logger::Allocations,
                                    tr("Sql error:\n%1").arg(query->lastError().text())
                                    );
    }
    else{
        QSqlQuery* update_query = new QSqlQuery(QSqlDatabase::database());
        while(query->next()){
            QSqlRecord r = query->record();
            if(!update_query->exec(QString("INSERT INTO allocations "
                                    "(`item_id`,`from_location_id`,`to_location_id`,`from_operator_id`,`to_operator_id`,`date`,`note`) "
                                    "VALUES ('%1', '%2', '%3', '%4', '%5', '%6', '%7')")
                            .arg(r.value("item_id").toInt())
                            .arg(null_id)
                            .arg((old_id == r.value("to_location_id").toInt()) ? null_id : r.value("to_location_id").toInt())
                            .arg(r.value("from_operator_id").toInt())
                            .arg(r.value("to_operator_id").toInt())
                            .arg(QDate::currentDate().toString("yyyy-MM-dd"))
                            .arg(tr("Automatic allocation. Deleted location:\nFrom: %1\nRevised allocation with id: %2,\nfrom date: %3,\nnote: %4")
                                 .arg(record.value("location").toString())
                                 .arg(r.value("id").toInt())
                                 .arg(r.value("date").toString())
                                 .arg(r.value("note").toString())
                                 )
                            )){
                if(logger) logger->writeLog(Logger::Error, Logger::Allocations,
                                            tr("Sql error:\n%1").arg(update_query->lastError().text())
                                            );
            }
            else{
                if(logger) logger->writeLog(Logger::Add, Logger::Allocations,
                                            tr("Automatic create allocation, copy of allocation:\n%1\nInserted id: %2")
                                                .arg(logger->infoLog(r))
                                                .arg(update_query->lastInsertId().toInt())
                                            );
            }
        }
    }
    if(!query->exec(QString("SELECT * FROM allocations WHERE `to_location_id`=%1")
                    .arg(old_id))){
        if(logger) logger->writeLog(Logger::Error, Logger::Allocations,
                                    tr("Sql error:\n%1").arg(query->lastError().text())
                                    );
    }
    else{
        QSqlQuery* update_query = new QSqlQuery(QSqlDatabase::database());
        while(query->next()){
            QSqlRecord r = query->record();
            if(!update_query->exec(QString("INSERT INTO allocations "
                                    "(`item_id`,`from_location_id`,`to_location_id`,`from_operator_id`,`to_operator_id`,`date`,`note`) "
                                    "VALUES ('%1', '%2', '%3', '%4', '%5', '%6', '%7')")
                            .arg(r.value("item_id").toInt())
                            .arg((old_id == r.value("from_location_id").toInt()) ? null_id : r.value("from_location_id").toInt())
                            .arg(null_id)
                            .arg(r.value("from_operator_id").toInt())
                            .arg(r.value("to_operator_id").toInt())
                            .arg(QDate::currentDate().toString("yyyy-MM-dd"))
                            .arg(tr("Automatic allocation. Deleted location:\nTo: %1\nRevised allocation with id: %2,\nfrom date: %3,\nnote: %4")
                                 .arg(record.value("location").toString())
                                 .arg(r.value("id").toInt())
                                 .arg(r.value("date").toString())
                                 .arg(r.value("note").toString())
                                  )
                             )){
                if(logger) logger->writeLog(Logger::Error, Logger::Allocations,
                                            tr("Sql error:\n%1").arg(update_query->lastError().text())
                                            );
            }
            else{
                if(logger) logger->writeLog(Logger::Edit, Logger::Allocations,
                                            tr("Automatic create allocation, copy of allocation:\n%1\nInserted id: %2")
                                            .arg(logger->infoLog(r))
                                            .arg(update_query->lastInsertId().toInt())
                                            );
            }
        }
    }
    if(!query->exec(QString("DELETE FROM allocations WHERE `to_location_id`=%1 OR `from_location_id`=%1")
                    .arg(old_id))){
        if(logger) logger->writeLog(Logger::Error, Logger::Allocations,
                                    tr("Sql error:\n%1").arg(query->lastError().text())
                                    );
    }
    else{
        if(logger) logger->writeLog(Logger::Delete, Logger::Allocations,
                                    tr("Delete allocations with location id: %1").arg(old_id)
                                    );
        emit tableUpdate("allocations");
    }
    this->save();
}

void TableForm::removeItemStatus(const QModelIndex &index)
{
    if(!this->checkUserRights(23)) return;
    QSqlRecord record = model->record(index.row());
    if(nullStatusId() == record.value("id").toInt()){
        QMessageBox msgBox;
        msgBox.setText(tr("Deletion of NULL item status is not allowed!"));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        return;
    }
    int ret = QMessageBox::question(this, tr("Item status"),
                                    tr("Do you want to delete %1 status?\nThis could not be restored!")
                                    .arg(record.value("status").toString()),
                                    QMessageBox::Ok | QMessageBox::Cancel);
    if(ret == QMessageBox::Cancel) return;
    int old_id = record.value("id").toInt();
    if(!model->removeRow(index.row())){
        if(logger) logger->writeLog(Logger::Error, Logger::Item_status,
                                    tr("Sql error:\n%1").arg(model->lastError().text())
                                    );
        return;
    }
    else{
        if(logger) logger->writeLog(Logger::Delete, Logger::Item_status, logger->infoLog(record));
    }
    this->save();
    QSqlQuery* query = new QSqlQuery(QSqlDatabase::database());
    int null_id = nullStatusId();
    if(!query->exec(QString("UPDATE items SET `status_id`=%1 WHERE `status_id`=%2")
                    .arg(null_id)
                    .arg(old_id))){
        if(logger) logger->writeLog(Logger::Error, Logger::Items,
                                    tr("Sql error:\n%1").arg(query->lastError().text())
                                    );
    }
    else{
        if(logger) logger->writeLog(Logger::Edit, Logger::Items,
                                    tr("Update items with deleted status id: %1").arg(old_id)
                                    );
    }
    this->save();
}

void TableForm::removeOperator(const QModelIndex &index)
{
    if(!this->checkUserRights(8)) return;
    QSqlRecord record = model->record(index.row());
    if(nullOperatorId() == record.value("id").toInt()){
        QMessageBox msgBox;
        msgBox.setText(tr("Deletion of NULL operator is not allowed!"));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        return;
    }
    int ret = QMessageBox::question(this, tr("Operators"),
                                    tr("Do you want to delete %1?\nThis could not be restored!")
                                    .arg(record.value("name").toString()),
                                    QMessageBox::Ok | QMessageBox::Cancel);
    if(ret == QMessageBox::Cancel) return;
    int old_id = record.value("id").toInt();
    if(!model->removeRow(index.row())){
        if(logger) logger->writeLog(Logger::Error, Logger::Operators,
                                    tr("Sql error:\n%1").arg(model->lastError().text())
                                    );
        return;
    }
    if(logger) logger->writeLog(Logger::Delete, Logger::Operators, logger->infoLog(record));
    this->save();
    QSqlQuery* query = new QSqlQuery(QSqlDatabase::database());
    int null_id = nullOperatorId();
    if(!query->exec(QString("UPDATE items SET `operator_id`=%1 WHERE `operator_id`=%2")
                    .arg(null_id)
                    .arg(old_id))){
        if(logger) logger->writeLog(Logger::Error, Logger::Items,
                                    tr("Sql error:\n%1").arg(query->lastError().text())
                                    );
    }
    else{
        if(logger) logger->writeLog(Logger::Edit, Logger::Items,
                                    tr("Update all items with operator id: %1").arg(old_id)
                                    );
    }
    if(!query->exec(QString("SELECT * FROM allocations WHERE `from_operator_id`=%1")
                    .arg(old_id))){
        if(logger) logger->writeLog(Logger::Error, Logger::Allocations,
                                    tr("Sql error:\n%1").arg(query->lastError().text())
                                    );
    }
    else{
        QSqlQuery* update_query = new QSqlQuery(QSqlDatabase::database());
        while(query->next()){
            QSqlRecord r = query->record();
            if(!update_query->exec(QString("INSERT INTO allocations "
                                    "(`item_id`,`from_location_id`,`to_location_id`,`from_operator_id`,`to_operator_id`,`date`,`note`) "
                                    "VALUES ('%1', '%2', '%3', '%4', '%5', '%6', '%7')")
                            .arg(r.value("item_id").toInt())
                            .arg(r.value("from_location_id").toInt())
                            .arg(r.value("to_location_id").toInt())
                            .arg(null_id)
                            .arg((old_id == r.value("to_operator_id").toInt()) ? null_id : r.value("to_operator_id").toInt())
                            .arg(QDate::currentDate().toString("yyyy-MM-dd"))
                            .arg(tr("Automatic allocation. Deleted operator:\nFrom: %1\nRevised allocation with id: %2,\nfrom date: %3,\nnote: %4")
                                 .arg(record.value("name").toString())
                                 .arg(r.value("id").toInt())
                                 .arg(r.value("date").toString())
                                 .arg(r.value("note").toString())
                                 )
                            )){
                if(logger) logger->writeLog(Logger::Error, Logger::Allocations,
                                            tr("Sql error:\n%1").arg(update_query->lastError().text())
                                            );
            }
            else{
                if(logger) logger->writeLog(Logger::Add, Logger::Allocations,
                                            tr("Automatic create allocation, copy of allocation:\n%1\nInserted id: %2")
                                            .arg(logger->infoLog(r))
                                            .arg(update_query->lastInsertId().toInt())
                                            );
            }
        }
    }
    if(!query->exec(QString("SELECT * FROM allocations WHERE `to_operator_id`=%1")
                    .arg(old_id))){
        if(logger) logger->writeLog(Logger::Error, Logger::Allocations,
                                    tr("Sql error:\n%1").arg(query->lastError().text())
                                    );
    }
    else{
        QSqlQuery* update_query = new QSqlQuery(QSqlDatabase::database());
        while(query->next()){
            QSqlRecord r = query->record();
            if(!update_query->exec(QString("INSERT INTO allocations "
                                    "(`item_id`,`from_location_id`,`to_location_id`,`from_operator_id`,`to_operator_id`,`date`,`note`) "
                                    "VALUES ('%1', '%2', '%3', '%4', '%5', '%6', '%7')")
                            .arg(r.value("item_id").toInt())
                            .arg(r.value("from_location_id").toInt())
                            .arg(r.value("to_location_id").toInt())
                            .arg((old_id == r.value("from_operator_id").toInt()) ? null_id : r.value("from_operator_id").toInt())
                            .arg(null_id)
                            .arg(QDate::currentDate().toString("yyyy-MM-dd"))
                            .arg(tr("Automatic allocation. Deleted operator:\nTo: %1\nRevised allocation with id: %2,\nfrom date: %3,\nnote: %4")
                                 .arg(record.value("name").toString())
                                 .arg(r.value("id").toInt())
                                 .arg(r.value("date").toString())
                                 .arg(r.value("note").toString())
                                  )
                             )){
                if(logger) logger->writeLog(Logger::Error, Logger::Allocations,
                                            tr("Sql error:\n%1").arg(update_query->lastError().text())
                                            );
            }
            else{
                if(logger) logger->writeLog(Logger::Add, Logger::Allocations,
                                            tr("Automatic create allocation, copy of allocation:\n%1\nInserted id: %2")
                                            .arg(logger->infoLog(r))
                                            .arg(update_query->lastInsertId().toInt())
                                            );
            }
        }
    }
    if(!query->exec(QString("DELETE FROM allocations WHERE `to_operator_id`=%1 OR `from_operator_id`=%1")
                    .arg(old_id))){
        if(logger) logger->writeLog(Logger::Error, Logger::Allocations,
                                    tr("Sql error:\n%1").arg(query->lastError().text())
                                    );
    }
    else
        if(logger) logger->writeLog(Logger::Delete, Logger::Allocations,
                                    tr("Delete allocations with deleted operator id: %1").arg(old_id)
                                    );
    emit tableUpdate("allocations");
    this->save();
}

void TableForm::removeDepartments(const QModelIndex &index)
{
    if(!this->checkUserRights(14)) return;
    QSqlRecord record = model->record(index.row());
    if(nullDepartmentId() == record.value("id").toInt()){
        QMessageBox msgBox;
        msgBox.setText(tr("Deletion of NULL department is not allowed!"));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        return;
    }
    int ret = QMessageBox::question(this, tr("Departments"),
                                    tr("Do you want to delete %1?\nThis could not be restored!")
                                    .arg(record.value("department").toString()),
                                    QMessageBox::Ok | QMessageBox::Cancel);
    if(ret == QMessageBox::Cancel) return;
    int old_id = record.value("id").toInt();
    if(!model->removeRow(index.row())){
        if(logger) logger->writeLog(Logger::Error, Logger::Departments,
                                    tr("Sql error:\n%1").arg(model->lastError().text())
                                    );
        return;
    }
    this->save();
    QSqlQuery* query = new QSqlQuery(QSqlDatabase::database());
    int null_id = nullDepartmentId();
    if(!query->exec(QString("UPDATE operators SET `department_id`=%1 WHERE `department_id`=%2")
                    .arg(null_id)
                    .arg(old_id))){
        if(logger) logger->writeLog(Logger::Error, Logger::Departments,
                                    tr("Sql error:\n%1").arg(query->lastError().text())
                                    );
    }
    else
        if(logger) logger->writeLog(Logger::Edit, Logger::Operators,
                                    tr("Update operators with deleted department id: %1").arg(old_id)
                                    );
    this->save();
}

void TableForm::clearFilter()
{
    ui->filter_comboBox->clearEditText();
    ui->field_comboBox->setCurrentIndex(0);
    saveFilterState();
}

int TableForm::nullLocationId() const
{
    QSqlQuery* query = new QSqlQuery(QSqlDatabase::database());
    int null_id = 0;
    if(!query->exec(QString("SELECT `id` FROM locations WHERE `location` LIKE 'N/A'"))){
        if(logger) logger->writeLog(Logger::Error, Logger::Locations,
                                    tr("Sql error:\n%1").arg(query->lastError().text())
                                    );
    }
    else if(!query->first()){
        qDebug() << "There are no unknown location.\nTry to create...";
        if(!query->exec(QString("INSERT INTO locations (`location`) VALUES ('N/A')"))){
            if(logger) logger->writeLog(Logger::Error, Logger::Locations,
                                        tr("Sql error:\n%1").arg(query->lastError().text())
                                        );
        }
        else null_id = query->lastInsertId().toInt();
    }
    else null_id = query->value(0).toInt();
    return null_id;
}

int TableForm::nullOperatorId() const
{
    QSqlQuery* query = new QSqlQuery(QSqlDatabase::database());
    int null_id = 0;
    if(!query->exec(QString("SELECT `id` FROM operators WHERE `name` LIKE 'N/A'"))){
        if(logger) logger->writeLog(Logger::Error, Logger::Operators,
                                    tr("Sql error:\n%1").arg(query->lastError().text())
                                    );
    }
    else if(!query->first()){
        qDebug() << "There are no unknown operator.\nTry to create...";
        if(!query->exec(QString("INSERT INTO operators (`name`,`department_id`) VALUES ('N/A','%1')")
                        .arg(nullDepartmentId()))){
            if(logger) logger->writeLog(Logger::Error, Logger::Operators,
                                        tr("Sql error:\n%1").arg(query->lastError().text())
                                        );
        }
        else null_id = query->lastInsertId().toInt();
    }
    else null_id = query->value(0).toInt();
    return null_id;
}

int TableForm::nullDepartmentId() const
{
    QSqlQuery* query = new QSqlQuery(QSqlDatabase::database());
    int null_id = 0;
    if(!query->exec(QString("SELECT `id` FROM departments WHERE `department` LIKE 'N/A'"))){
        if(logger) logger->writeLog(Logger::Error, Logger::Departments,
                                    tr("Sql error:\n%1").arg(query->lastError().text())
                                    );
    }
    else if(!query->first()){
        qDebug() << "There are no unknown department.\nTry to create...";
        if(!query->exec(QString("INSERT INTO departments (`department`) VALUES ('N/A')"))){
            if(logger) logger->writeLog(Logger::Error, Logger::Departments,
                                        tr("Sql error:\n%1").arg(query->lastError().text())
                                        );
        }
        else null_id = query->lastInsertId().toInt();
    }
    else null_id = query->value(0).toInt();
    return null_id;
}

int TableForm::nullStatusId() const
{
    QSqlQuery* query = new QSqlQuery(QSqlDatabase::database());
    int null_id = 0;
    if(!query->exec(QString("SELECT `id` FROM item_status WHERE `status` LIKE 'N/A'"))){
        if(logger) logger->writeLog(Logger::Error, Logger::Item_status,
                                    tr("Sql error:\n%1").arg(query->lastError().text())
                                    );
    }
    else if(!query->first()){
        qDebug() << "There are no unknown status.\nTry to create...";
        if(!query->exec(QString("INSERT INTO item_status (`status`) VALUES ('N/A')"))){
            if(logger) logger->writeLog(Logger::Error, Logger::Item_status,
                                        tr("Sql error:\n%1").arg(query->lastError().text())
                                        );
        }
        else null_id = query->lastInsertId().toInt();
    }
    else null_id = query->value(0).toInt();
    return null_id;
}

int TableForm::nullTypeId() const
{
    QSqlQuery* query = new QSqlQuery(QSqlDatabase::database());
    int null_id = 0;
    if(!query->exec(QString("SELECT `id` FROM item_types WHERE `type` LIKE 'N/A'"))){
        if(logger) logger->writeLog(Logger::Error, Logger::Item_types,
                                    tr("Sql error:\n%1").arg(query->lastError().text())
                                    );
    }
    else if(!query->first()){
        qDebug() << "There are no unknown type.\nTry to create...";
        if(!query->exec(QString("INSERT INTO item_types (`type`) VALUES ('N/A')"))){
            if(logger) logger->writeLog(Logger::Error, Logger::Item_types,
                                        tr("Sql error:\n%1").arg(query->lastError().text())
                                        );
        }
        else null_id = query->lastInsertId().toInt();
    }
    else null_id = query->value(0).toInt();
    return null_id;
}

int TableForm::nullItemId() const
{
    QSqlQuery* query = new QSqlQuery(QSqlDatabase::database());
    int null_id = 0;
    if(!query->exec(QString("SELECT `id` FROM items WHERE `note` LIKE 'N/A' "
                            "AND `number` LIKE 'N/A' AND `model` LIKE 'N/A' "
                            "AND `status_id`=%1 AND `type_id`=%2 AND `location_id`=%3 "
                            "AND `operator_id`=%4")
                    .arg(nullStatusId())
                    .arg(nullTypeId())
                    .arg(nullLocationId())
                    .arg(nullOperatorId())
                    )){
        if(logger) logger->writeLog(Logger::Error, Logger::Items,
                                    tr("Sql error:\n%1").arg(query->lastError().text())
                                    );
    }
    else if(!query->first()){
        qDebug() << "There are no unknown item.\nTry to create...";
        if(!query->exec(QString("INSERT INTO items (`note`,`number`,`model`,`status_id`,`type_id`,"
                                "`operator_id`,`location_id`) VALUES ('N/A','N/A','N/A','%1','%2','%3','%4')")
                        .arg(nullStatusId())
                        .arg(nullTypeId())
                        .arg(nullOperatorId())
                        .arg(nullLocationId())
                        )){
            if(logger) logger->writeLog(Logger::Error, Logger::Items,
                                        tr("Sql error:\n%1").arg(query->lastError().text())
                                        );
        }
        else null_id = query->lastInsertId().toInt();
    }
    else null_id = query->value(0).toInt();
    return null_id;
}

void TableForm::exportToCsv()
{
    if(!model) return;
    QString csv = QFileDialog::getSaveFileName(this, tr("Export CSV"),
    QDesktopServices::storageLocation(QDesktopServices::DesktopLocation)+"/"+model->tableName()+"_export.csv",
    tr("Comma Separated Values (*.csv)"));

    int x = 0;
    QString exportdata;

    int counthidden=0, jint = 0;

    while (jint < model->columnCount()) {
        counthidden+=ui->tableView->isColumnHidden(jint);
        jint++;
    }

    int w = 1;
    while (x < model->columnCount()){
        if (!ui->tableView->isColumnHidden(x)) {
            exportdata.append('\"'+model->headerData(x,Qt::Horizontal,Qt::DisplayRole).toString()+'\"');
            if (model->columnCount() - w > counthidden)
                exportdata.append(',');
            else {
                exportdata.append('\n');
            }
            w++;
        }
        x++;
    }
    int z = 0;
    w = 1;
    while (z < model->rowCount()) {
        x = 0;
        w = 1;
        while (x < model->columnCount()) {
            if (!ui->tableView->isColumnHidden(x)) {
                exportdata.append('\"'+QString(model->data(model->index(z,x),Qt::DisplayRole).toString())/*.replace('\n','|')*/+'\"');
                if (model->columnCount() - w > counthidden)
                    exportdata.append(',');
                else
                    exportdata.append('\n');
                w++;
            }
            x++;
        }
        z++;
    }

    QFile file;
    if (!csv.isEmpty()) {
        file.setFileName(csv);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
            return;
    }

    QTextCodec *codec = QTextCodec::codecForName("CP1251");
    file.write(codec->fromUnicode(exportdata));
    file.close();

    QUrl url;
    QString filePath = file.fileName();
    if (filePath.startsWith("\\\\") || filePath.startsWith("//"))
    url.setUrl(QDir::toNativeSeparators(filePath));
    else
    url = QUrl::fromLocalFile(filePath);
    if(!QDesktopServices::openUrl(url)){
        qDebug() << "Error with document opening. The url path: " << url.path();
        qDebug() << "Absolute file path: " << filePath;
    }
}

void TableForm::refresh()
{
    if(model)
        needUpdate(model->tableName());
}

void TableForm::removeLogs(const QList<QModelIndex> &list)
{
    if(!this->checkUserRights(28)) return;
    int ret = QMessageBox::question(this, tr("Logs"),
                                tr("Do you want to delete this %1 logs?\nThis could not be restored!")
                                .arg(list.count()),
                                QMessageBox::Ok | QMessageBox::Cancel);
    if(ret == QMessageBox::Cancel) return;
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    if(list.isEmpty()) return;
    QString ids;
    for (int i = 0; i < list.size(); ++i) {
        ids.append(list.at(i).data(Qt::EditRole).toString());
        if(list.at(i) != list.last()) ids.append(',');
    }
    QSqlQuery* query = new QSqlQuery(QSqlDatabase::database());
    if(!query->exec(QString("DELETE from logs WHERE `id` IN (%1)")
                    .arg(ids))){
            if(logger) logger->writeLog(Logger::Error, Logger::Logs,
                                        tr("Sql error:\n%1").arg(query->lastError().text())
                                        );
    }
    QApplication::restoreOverrideCursor();
    refresh();
}

void TableForm::showHeaderId(bool show)
{
    emit showId(show);
}

void TableForm::setLogger(Logger *l)
{
    logger = l;
    ui->tableView->setLogger(l);
    ui->detailed_tableView->setLogger(l);
}

void TableForm::findItem(const int item_id)
{
    if(!model) return;
    while (model->canFetchMore())
         model->fetchMore();
    QModelIndexList Items = model->match(model->index( 0, model->fieldIndex("id")), Qt::DisplayRole, item_id, 1, Qt::MatchExactly);
    if(Items.isEmpty()){
        QMessageBox::information ( this, windowTitle(), tr("The search did not return results"));
        return;
    }
    ui->tableView->selectionModel()->setCurrentIndex(Items.at(0),
                                                            QItemSelectionModel::Toggle | QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);
    int visible_column = 0;
    while(ui->tableView->horizontalHeader()->isSectionHidden(visible_column++)){
        if(ui->tableView->horizontalHeader()->count() >= visible_column) break;
    }
    ui->tableView->scrollTo(model->index(Items.at(0).row(), visible_column));
    ui->tableView->setFocus();
}

QString TableForm::transFilterField(const QString& field, const bool direction) const
{
/* bool from - translation direction
  true - from source to translation
  false - from translation to source
  */
    QString tr_field = field;
    if(direction){
        //items
        if(field == "status") tr_field = tr("Status");
        else if(field == "note") tr_field = tr("Note");
        else if(field == "location") tr_field = tr("Location");
        else if(field == "model") tr_field = tr("Model");
        else if(field == "manufacturer") tr_field = tr("Manufacturer");
        else if(field == "serialno") tr_field = tr("Serial No");
        else if(field == "type") tr_field = tr("Type");
        else if(field == "number") tr_field = tr("Number");
        else if(field == "name") tr_field = tr("Name");
        //users
        else if(field == "username") tr_field = tr("User");
        else if(field == "firstname") tr_field = tr("First name");
        else if(field == "lastname") tr_field = tr("Last name");
        else if(field == "email") tr_field = tr("E-mail");
        //departments
        else if(field == "department") tr_field = tr("Department");
        //locations
        else if(field == "date") tr_field = tr("Date");
        else if(field == "old_location") tr_field = tr("Old location");
        else if(field == "old_name") tr_field = tr("Old name");
    }
    else{
        //items
        if(field == tr("Status")) tr_field = "status";
        else if(field == tr("Note")) tr_field = "note";
        else if(field == tr("Location")) tr_field = "location";
        else if(field == tr("Model")) tr_field = "model";
        else if(field == tr("Manufacturer")) tr_field = "manufacturer";
        else if(field == tr("Serial No")) tr_field = "serialno";
        else if(field == tr("Type")) tr_field = "type";
        else if(field == tr("Number")) tr_field = "number";
        else if(field == tr("Name")) tr_field = "name";
        //users
        else if(field == tr("User")) tr_field = "username";
        else if(field == tr("First name")) tr_field = "firstname";
        else if(field == tr("E-mail")) tr_field = "email";
        else if(field == tr("Last name")) tr_field = "lastname";
        //departments
        else if(field == tr("Department")) tr_field = "department";
        //locations
        else if(field == tr("Date")) tr_field = "date";
        else if(field == tr("Old location")) tr_field = "old_location";
        else if(field == tr("Old name")) tr_field = "old_name";
    }
    return tr_field;
}

void TableForm::saveFilterState()
{
    last_filter_field = ui->field_comboBox->currentIndex();
    last_filter = ui->filter_comboBox->currentText();
}

void TableForm::restoreFilterState()
{
    if(!last_filter_field.isEmpty()){
        if(ui->field_comboBox->findText(last_filter_field) >= 0) ui->field_comboBox->setCurrentIndex(ui->field_comboBox->findText(last_filter_field));
        else ui->field_comboBox->setCurrentIndex(0);
    }
    if(!last_filter.isEmpty()){
        if(ui->filter_comboBox->findText(last_filter) >= 0) ui->filter_comboBox->setCurrentIndex(ui->filter_comboBox->findText(last_filter));
        else ui->filter_comboBox->insertItem(0, last_filter);
    }
}

void TableForm::updateFields()
{
    saveFilterState();
    fillFilterFields();
    restoreFilterState();
}

void TableForm::sortIndicatorChanged(int logicalIndex, Qt::SortOrder order)
{
    ui->tableView->restoreMarked();
}

/* properties addon */
void TableForm::showTypeProperties(const bool show)
{
    if(show_attachments){
        show_attachments->blockSignals(true);
        show_attachments->setChecked(false);
        show_attachments->blockSignals(false);
    }
    if(show_items){
        show_items->blockSignals(true);
        show_items->setChecked(false);
        show_items->blockSignals(false);
    }
    if(show_rights){
        show_rights->blockSignals(true);
        show_rights->setChecked(false);
        show_rights->blockSignals(false);
    }
    if(!show){
        if(ui->detailed_toolButton->isChecked()) ui->detailed_toolButton->toggle();
        return;
    }
    if(!this->checkUserRights(18)) return;

    if(!ui->detailed_toolButton->isChecked()) ui->detailed_toolButton->toggle();
    detailed_filter = QString("type_id = ");

    if(!detailed_model) detailed_model = new MySqlRelationalTableModel(ui->detailed_tableView);
    detailed_model->clear();
    detailed_model->setTable("properties");
    detailed_model->select();
    ui->detailed_tableView->setModel(detailed_model);
    ui->detailed_tableView->hideColumn(detailed_model->fieldIndex("id"));
    ui->detailed_tableView->hideColumn(detailed_model->fieldIndex("type_id"));

    //ui->detailed_tableView->setItemDelegate(new QSqlRelationalDelegate(ui->tableView));

    detailed_model->setHeaderData(detailed_model->fieldIndex("property"), Qt::Horizontal, tr("Properties"), Qt::EditRole);

    this->updateDetailedView();
}

void TableForm::addProperty()
{
    if(model){
        QSqlRecord record = model->record(current_row);
        if(!record.isEmpty()){
            bool ok;
            QString text = QInputDialog::getText(this, tr("%1 (new property)").arg(record.value("type").toString()),
                                                 tr("Property name:"), QLineEdit::Normal,
                                                 "", &ok);
            if (ok && !text.isEmpty()){
                QSqlQuery* query = new QSqlQuery(QSqlDatabase::database());
                if(!query->exec(QString("INSERT INTO properties (`type_id`, `property`) VALUES ('%1', '%2')")
                                .arg(record.value("id").toInt())
                                .arg(text))
                   ){
                    if(logger) logger->writeLog(Logger::Error, Logger::Properties,
                                            tr("Sql error:\n%1").arg(query->lastError().text())
                                            );
                    }
                    else{
                        if(logger) logger->writeLog(Logger::Add, Logger::Properties,
                                                tr("Property: %1 has been added to:\n%2")
                                                .arg(text)
                                                .arg(logger->infoLog(record))
                                                );
                    }
                    this->updateDetailedView();
            }
        }
    }
}

void TableForm::showItemProperties(const bool show)
{
    if(show_attachments){
        show_attachments->blockSignals(true);
        show_attachments->setChecked(false);
        show_attachments->blockSignals(false);
    }
    if(show_items){
        show_items->blockSignals(true);
        show_items->setChecked(false);
        show_items->blockSignals(false);
    }
    if(show_rights){
        show_rights->blockSignals(true);
        show_rights->setChecked(false);
        show_rights->blockSignals(false);
    }
    if(!show){
        if(ui->detailed_toolButton->isChecked()) ui->detailed_toolButton->toggle();
        return;
    }
    if(!this->checkUserRights(18)) return;

    if(!ui->detailed_toolButton->isChecked()) ui->detailed_toolButton->toggle();
    detailed_filter = QString("item_id = ");

    if(!detailed_model) detailed_model = new MySqlRelationalTableModel(ui->detailed_tableView);
    detailed_model->clear();
    detailed_model->setTable("item_properties");
    detailed_model->setRelation(detailed_model->fieldIndex("property_id"), QSqlRelation("properties", "id", "property"));
    detailed_model->select();
    ui->detailed_tableView->setModel(detailed_model);
    ui->detailed_tableView->hideColumn(detailed_model->fieldIndex("id"));
    ui->detailed_tableView->hideColumn(detailed_model->fieldIndex("item_id"));

    //ui->detailed_tableView->setItemDelegate(new QSqlRelationalDelegate(ui->tableView));

    detailed_model->setHeaderData(detailed_model->fieldIndex("property"), Qt::Horizontal, tr("Property"), Qt::EditRole);
    detailed_model->setHeaderData(detailed_model->fieldIndex("value"), Qt::Horizontal, tr("Value"), Qt::EditRole);

    this->updateDetailedView();
}

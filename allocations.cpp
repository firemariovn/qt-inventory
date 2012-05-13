#include "allocations.h"
#include "ui_allocations.h"
#include "logger.h"

#include <QtSql>
#include <QtGui>

Allocations::Allocations(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Allocations)
{
    ui->setupUi(this);
    ui->dateEdit->setDateTime(QDateTime::currentDateTime());

    setEditMode(false);
    setAllocationId(0);
    setItemId(0);

    ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(false);

    items_model = 0;
    locations_model = 0;
    operators_model = 0;
    logger = 0;

    loadItems();
    loadLocations();
    loadOperators();

    ui->to_operator_comboBox->completer()->setCompletionMode(QCompleter::PopupCompletion);
    ui->to_location_comboBox->completer()->setCompletionMode(QCompleter::PopupCompletion);

    connect(ui->filter_lineEdit, SIGNAL(textChanged(QString)), this, SLOT(setItemsFilter(QString)));
}

Allocations::~Allocations()
{
    delete ui;
}

void Allocations::loadAllocation(const int allocation_id)
{
    QSqlQuery* query = new QSqlQuery(QSqlDatabase::database());
    if(!query->exec(QString("SELECT * FROM allocations WHERE `id`=%1")
                        .arg(allocation_id))){
        qDebug() << "Can not get allocation data from database. Sql error: " << query->lastError().text();
        if(logger) logger->writeLog(Logger::Error, Logger::Allocations,
                                    tr("Can not get allocation data from database.\n%1").arg(query->lastError().text())
                                    );
        return;
    }
    if(!query->first()){
        qDebug() << "No allocation with id: " << allocation_id;
        return;
    }
    setAllocationId(allocation_id);
    QSqlRecord record = query->record();
    setItemFilter(record.value("item_id").toInt());
    setLocationFilter(record.value("to_location_id").toInt());
    setOperatorFilter(record.value("to_operator_id").toInt());
    ui->dateEdit->setDateTime(QDateTime::fromString(record.value("date").toString(), "yyyy-MM-dd HH:mm"));
    ui->note_textEdit->setText(record.value("note").toString());
    //fill from_location field into dialog
    if(!query->exec(QString("SELECT `location` FROM locations WHERE `id`=%1")
                    .arg(record.value("from_location_id").toInt()))){
        qDebug() << "Can not get from_location data from database. Sql error: " << query->lastError().text();
        if(logger) logger->writeLog(Logger::Error, Logger::Allocations,
                                    tr("Can not get from_location data from database.\n%1").arg(query->lastError().text())
                                    );
        return;
    }
    if(!query->first()){
        qDebug() << "No location with this id: " << allocation_id;
        return;
    }
    else ui->from_location_lineEdit->setText(query->value(0).toString());
    //fill from_operator field into dialog
    if(!query->exec(QString("SELECT `name` FROM operators WHERE `id`=%1")
                    .arg(record.value("from_operator_id").toInt()))){
        qDebug() << "Can not get from_operator data from database. Sql error: " << query->lastError().text();
        if(logger) logger->writeLog(Logger::Error, Logger::Allocations,
                                    tr("Can not get from_operator data from database.\n%1").arg(query->lastError().text())
                                    );
        return;
    }
    if(!query->first()){
        qDebug() << "No operator with this id: " << allocation_id;
        return;
    }
    else ui->from_operator_lineEdit->setText(query->value(0).toString());
    if(logger) logger->writeLog(Logger::View, Logger::Allocations,
                                tr("Edit allocation with id: %1").arg(allocation_id)
                                );
}

void Allocations::loadItems()
{
    if(!items_model) items_model = new QSqlRelationalTableModel(ui->item_treeView);
    items_model->clear();
    items_model->setTable("items");
    items_model->setRelation(items_model->fieldIndex("location_id"), QSqlRelation("locations", "id", "location"));
    items_model->setRelation(items_model->fieldIndex("type_id"), QSqlRelation("item_types", "id", "type"));
    items_model->setRelation(items_model->fieldIndex("operator_id"), QSqlRelation("operators", "id", "name"));
    items_model->setRelation(items_model->fieldIndex("status_id"), QSqlRelation("item_status", "id", "status"));
    items_model->select();
    this->setItemsFilter("");

    ui->item_treeView->setModel(items_model);
    ui->item_treeView->hideColumn(items_model->fieldIndex("id"));

    items_model->setHeaderData(items_model->fieldIndex("type"), Qt::Horizontal, tr("Type"), Qt::EditRole);
    items_model->setHeaderData(items_model->fieldIndex("status"), Qt::Horizontal, tr("Status"), Qt::EditRole);
    items_model->setHeaderData(items_model->fieldIndex("location"), Qt::Horizontal, tr("Location"), Qt::EditRole);
    items_model->setHeaderData(items_model->fieldIndex("IP"), Qt::Horizontal, tr("IP address"), Qt::EditRole);
    items_model->setHeaderData(items_model->fieldIndex("note"), Qt::Horizontal, tr("Note"), Qt::EditRole);
    items_model->setHeaderData(items_model->fieldIndex("model"), Qt::Horizontal, tr("Model"), Qt::EditRole);
    items_model->setHeaderData(items_model->fieldIndex("manufacturer"), Qt::Horizontal, tr("Manufacturer"), Qt::EditRole);
    items_model->setHeaderData(items_model->fieldIndex("serialno"), Qt::Horizontal, tr("Serial No"), Qt::EditRole);
    items_model->setHeaderData(items_model->fieldIndex("number"), Qt::Horizontal, tr("Number"), Qt::EditRole);
    items_model->setHeaderData(items_model->fieldIndex("name"), Qt::Horizontal, tr("Name"), Qt::EditRole);

    connect(ui->item_treeView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(itemSelected(QModelIndex,QModelIndex)));
}

void Allocations::setItemFilter(const int item_id)
{
    setItemId(item_id);
    items_model->setFilter(QString("items.id = %1").arg(item_id));
    ui->item_treeView->selectionModel()->setCurrentIndex(items_model->index(0,0), QItemSelectionModel::Toggle | QItemSelectionModel::Rows);
}

void Allocations::setLocationFilter(const int location_id)
{
    //locations_model->setFilter(QString("locations.id = %1").arg(location_id));
    QModelIndexList Locations = locations_model->match(locations_model->index( 0, locations_model->fieldIndex("id")), Qt::DisplayRole, location_id);
    ui->to_location_comboBox->setCurrentIndex(Locations[0].row());
}

void Allocations::setOperatorFilter(const int operator_id)
{
    //operators_model->setFilter(QString("operators.id = %1").arg(operator_id));
    QModelIndexList Operators = operators_model->match(operators_model->index( 0, operators_model->fieldIndex("id")), Qt::DisplayRole, operator_id);
    ui->to_operator_comboBox->setCurrentIndex(Operators[0].row());
}

void Allocations::loadLocations()
{
    if(!locations_model) locations_model = new QSqlTableModel(ui->to_location_comboBox);
    locations_model->clear();
    locations_model->setTable("locations");
    locations_model->select();
    ui->to_location_comboBox->setModel(locations_model);
    ui->to_location_comboBox->setModelColumn(locations_model->fieldIndex("location"));
}

void Allocations::loadOperators()
{
    if(!operators_model) operators_model = new QSqlTableModel(ui->to_operator_comboBox);
    operators_model->clear();
    operators_model->setTable("operators");
    operators_model->select();
    ui->to_operator_comboBox->setModel(operators_model);
    ui->to_operator_comboBox->setModelColumn(operators_model->fieldIndex("name"));
}

void Allocations::itemSelected(const QModelIndex &current, const QModelIndex &previous)
{
    ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(current.isValid());
    if(isEditMode()) return;
    if(current.isValid()){     
        if(items_model){
            QSqlRecord record = items_model->record(current.row());
            if(!record.isEmpty()){
                ui->from_location_lineEdit->setText(record.value("location").toString());
                ui->from_operator_lineEdit->setText(record.value("name").toString());
            }
            else{
                //current_id = 0;
            }
        }
    }
    else{
        ui->from_location_lineEdit->clear();
        ui->from_operator_lineEdit->clear();
    }
}

void Allocations::setItemsFilter(const QString &filter)
{
    if(!items_model) return;
    if(filter.isEmpty()){
        items_model->setFilter(filter);
    }
    else{
        items_model->setFilter(QString("status LIKE '%%1%' OR "
                                 "items.IP LIKE '%%1%' OR "
                                 "items.model LIKE '%%1%' OR "
                                 "items.note LIKE '%%1%' OR "
                                 "items.manufacturer LIKE '%%1%' OR "
                                 "items.serialno LIKE '%%1%' OR "
                                 "name LIKE '%%1%' OR "
                                 "location LIKE '%%1%' OR "
                                 "type LIKE '%%1%' OR "
                                 "items.number LIKE '%%1%'"
                                 )
                               .arg(filter)
                                            );
       // qDebug() << "Items filter: " << items_model->filter();
       // qDebug() << items_model->lastError().text();
    }
}

void Allocations::setEditMode(const bool edit)
{
    this->edit_mode = edit;
    setWindowTitle(tr("Allocations (%1 allocation)").arg(edit ? tr("edit") : tr("new")));
    ui->filter_lineEdit->clear();
    ui->filter_lineEdit->setDisabled(edit);
}

void Allocations::accept()
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    if(!saveAllocation()){
        QApplication::restoreOverrideCursor();
        return;
    }
    QApplication::restoreOverrideCursor();
    QDialog::accept();
}

bool Allocations::saveAllocation()
{
    if(ui->to_operator_comboBox->currentText() != ui->to_operator_comboBox->itemModelText() || ui->to_operator_comboBox->itemModelId() < 1){
        ui->to_operator_comboBox->showPopup(2);
        ui->to_operator_comboBox->showTip( tr("Please, choose a valid operator"));
        return false;
    }
    if(ui->to_location_comboBox->currentText() != ui->to_location_comboBox->itemModelText() || ui->to_location_comboBox->itemModelId() < 1){
        ui->to_location_comboBox->showPopup(2);
        ui->to_location_comboBox->showTip(tr("Please, choose a valid location"));
        return false;
    }
    if(!(ui->to_location_comboBox->checkData() & ui->to_operator_comboBox->checkData()
         )){
        int ret = QMessageBox::question(0, tr("Allocations"),
                                        tr("You have blank fields.\n"
                                           "Do you want to continue?\n"),
                                            QMessageBox::Ok | QMessageBox::Cancel);
        if(ret == QMessageBox::Cancel) return false;
    }
    if(isEditMode()){
        if(!getAllocationId()){
            qDebug() << "Unknown allocation id!";
            return false;
        }
        if(!locations_model || !operators_model){
            qDebug() << "Incomplete data for saving. Interrupt ...";
            return false;
        }
        QString info;
        QSqlQuery* query = new QSqlQuery(QSqlDatabase::database());
        if(logger){
            if(!query->exec(QString("SELECT * FROM allocations WHERE `id`=%1").arg(getAllocationId()))){
                logger->writeLog(Logger::Error, Logger::Allocations,
                                 tr("Can not load allocation data.\n%1").arg(query->lastError().text())
                                 );
            }
            else info = logger->infoLog(query);
        }
        if(!query->exec(QString("UPDATE allocations SET `to_location_id`='%1',`to_operator_id`='%2',"
                                "`date`='%3',`note`='%4' WHERE `id`='%5'")
                        .arg(locations_model->data(locations_model->index(ui->to_location_comboBox->currentIndex(), locations_model->fieldIndex("id"))).toInt())
                        .arg(operators_model->data(operators_model->index(ui->to_operator_comboBox->currentIndex(), operators_model->fieldIndex("id"))).toInt())
                        .arg(ui->dateEdit->dateTime().toString("yyyy-MM-dd HH:mm"))
                        .arg(ui->note_textEdit->toPlainText())
                        .arg(getAllocationId())
                        )){
            if(logger) logger->writeLog(Logger::Error, Logger::Allocations,
                                        tr("Can not save data.\n%1").arg(query->lastError().text())
                                        );
            return false;
        }
        if(!query->exec(QString("UPDATE items SET `location_id`='%1',"
                                "`operator_id`='%2' WHERE `id`='%3'")
                        .arg(locations_model->data(locations_model->index(ui->to_location_comboBox->currentIndex(), locations_model->fieldIndex("id"))).toInt())
                        .arg(operators_model->data(operators_model->index(ui->to_operator_comboBox->currentIndex(), operators_model->fieldIndex("id"))).toInt())
                        .arg(getItemId())
                        )){
            if(logger) logger->writeLog(Logger::Error, Logger::Allocations,
                                        tr("Can not save item data.\n%1").arg(query->lastError().text())
                                        );
            return false;
        }
        if(logger) logger->writeLog(Logger::Edit, Logger::Allocations,
                                    tr("Save changes into allocation.\nOld data:\n%1")
                                    .arg(info)
                                    );

    }
    else{
        if(!items_model || !locations_model || !operators_model){
            qDebug() << "Incomplete data for saving. Interrupt ...";
            return false;
        }
        QModelIndex itemIndex = ui->item_treeView->selectionModel()->currentIndex();
        if(!itemIndex.isValid()){
            return false;
        }
        QSqlRecord record = items_model->record(itemIndex.row());
        QSqlQuery* query = new QSqlQuery(QSqlDatabase::database());
        if(!query->exec(QString("SELECT `operator_id`, `location_id` FROM items WHERE `id`=%1")
                        .arg(record.value("id").toInt()))){
            if(logger) logger->writeLog(Logger::Error, Logger::Allocations,
                                        tr("Can not get item data from database.\n%1").arg(query->lastError().text())
                                        );
            return false;
        }
        if(!query->first()){
            qDebug() << "No item with id: " << record.value("id").toInt();
            return false;
        }
        QSqlRecord itemRecord = query->record();
        if(!query->exec(QString("INSERT INTO allocations (`item_id`,`from_operator_id`,`from_location_id`,`to_operator_id`,`to_location_id`,`date`,`note`) "
                                "VALUES ('%1', '%2', '%3', '%4', '%5', '%6', '%7')")
                        .arg(record.value("id").toInt())
                        .arg(itemRecord.value("operator_id").toInt())
                        .arg(itemRecord.value("location_id").toInt())
                        .arg(operators_model->data(operators_model->index(ui->to_operator_comboBox->currentIndex(), operators_model->fieldIndex("id"))).toInt())
                        .arg(locations_model->data(locations_model->index(ui->to_location_comboBox->currentIndex(), locations_model->fieldIndex("id"))).toInt())
                        .arg(ui->dateEdit->dateTime().toString("yyyy-MM-dd HH:mm"))
                        .arg(ui->note_textEdit->toPlainText())
                        )){
            if(logger) logger->writeLog(Logger::Error, Logger::Allocations,
                                        tr("Can not save data.\n%1").arg(query->lastError().text())
                                        );
            return false;
        }
        if(!query->exec(QString("UPDATE items SET `location_id`='%1',"
                                "`operator_id`='%2' WHERE `id`='%3'")
                        .arg(locations_model->data(locations_model->index(ui->to_location_comboBox->currentIndex(), locations_model->fieldIndex("id"))).toInt())
                        .arg(operators_model->data(operators_model->index(ui->to_operator_comboBox->currentIndex(), operators_model->fieldIndex("id"))).toInt())
                        .arg(record.value("id").toInt())
                        )){
            if(logger) logger->writeLog(Logger::Error, Logger::Allocations,
                                        tr("Can not save data.\n%1").arg(query->lastError().text())
                                        );
        }
        if(logger) logger->writeLog(Logger::Add, Logger::Allocations, tr("Defined allocation with id: %1")
                                    .arg(query->lastInsertId().toInt())
                                    );
    }
    return true;
}

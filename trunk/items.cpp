#include "items.h"
#include "ui_items.h"
#include "logger.h"
#include "propertyitemdelegate.h"
#include "attachmentsview.h"

#include <QtSql>
#include <QtGui>
#include <QDir>

Items::Items(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Items)
{
    ui->setupUi(this);

    locations_model = 0;
    operators_model = 0;
    types_model = 0;
    status_model = 0;
    logger = 0;

    ui->operator_comboBox->completer()->setCompletionMode(QCompleter::PopupCompletion);
    ui->operator_comboBox->completer()->setModelSorting(QCompleter::CaseSensitivelySortedModel);
    ui->operator_comboBox->completer()->setCaseSensitivity(Qt::CaseInsensitive);
    ui->location_comboBox->completer()->setCompletionMode(QCompleter::PopupCompletion);
    ui->location_comboBox->completer()->setModelSorting(QCompleter::CaseSensitivelySortedModel);
    ui->location_comboBox->completer()->setCaseSensitivity(Qt::CaseInsensitive);

    setEditMode(false);
    setLineEditCompleter(ui->manufacturer_lineEdit, "manufacturer");
    setLineEditCompleter(ui->model_lineEdit, "model");
    loadData();
}

Items::~Items()
{
    delete ui;
}

void Items::accept()
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    if(!saveItem()){
        QApplication::restoreOverrideCursor();
        return;
    }
    this->saveItemProperties();
    this->saveItemAttachments();
    QApplication::restoreOverrideCursor();
    QDialog::accept();
}

bool Items::saveItem() const
{
    if(ui->type_comboBox->currentText() != ui->type_comboBox->itemModelText() || ui->type_comboBox->itemModelId() < 1){
        ui->type_comboBox->showPopup(2);
        ui->type_comboBox->showTip(tr("Please, choose a valid item type"));
        return false;
    }
    if(ui->operator_comboBox->currentText() != ui->operator_comboBox->itemModelText() || ui->operator_comboBox->itemModelId() < 1){
        ui->operator_comboBox->showPopup(2);
        ui->operator_comboBox->showTip( tr("Please, choose a valid operator"));
        return false;
    }
    if(ui->location_comboBox->currentText() != ui->location_comboBox->itemModelText() || ui->location_comboBox->itemModelId() < 1){
        ui->location_comboBox->showPopup(2);
        ui->location_comboBox->showTip(tr("Please, choose a valid location"));
        return false;
    }
    if(ui->status_comboBox->currentText() != ui->status_comboBox->itemModelText() || ui->status_comboBox->itemModelId() < 1){
        ui->status_comboBox->showPopup(2);
        ui->status_comboBox->showTip(tr("Please, choose a valid item status"));
        return false;
    }
    if(!checkNumber()) return false;
    if(!(ui->type_comboBox->checkData() & ui->operator_comboBox->checkData()
            & ui->location_comboBox->checkData() & ui->status_comboBox->checkData()
            & ui->number_lineEdit->checkData() & ui->ipaddress_lineEdit->checkData()
         )){
        int ret = QMessageBox::question(0, tr("Items"),
                                        tr("You have blank fields.\n"
                                           "Do you want to continue?"),
                                            QMessageBox::Ok | QMessageBox::Cancel);
        if(ret == QMessageBox::Cancel) return false;
    }
    //int new_operator_id = operators_model->data(operators_model->index(ui->operator_comboBox->currentIndex(), operators_model->fieldIndex("id"))).toInt();
    int new_operator_id = ui->operator_comboBox->itemModelId();
    int new_location_id = ui->location_comboBox->itemModelId();
    //int new_location_id = locations_model->data(locations_model->index(ui->location_comboBox->currentIndex(), locations_model->fieldIndex("id"))).toInt();
    if(!isEditMode()){
        if(new_operator_id != getOperatorId() || new_location_id != getLocationId()){
            int ret = QMessageBox::question(0, tr("Items"),
                                            tr("You have defined a new item.\n"
                                               "Must be performed new, automatic allocation.\n"
                                               "Do you want to continue?\n"
                                               "This could not be restored!"),
                                                QMessageBox::Ok | QMessageBox::Cancel);
            if(ret == QMessageBox::Cancel) return false;
        }
        QSqlQuery* query = new QSqlQuery(QSqlDatabase::database());
        if(!query->exec(QString("INSERT INTO items (`type_id`,`operator_id`,`location_id`,`status_id`) "
                                "VALUES ('%1', '%2', '%3', '%4')")
                        .arg(types_model->data(types_model->index(ui->type_comboBox->currentIndex(), types_model->fieldIndex("id"))).toInt())
                        .arg(new_operator_id)
                        .arg(new_location_id)
                        .arg(status_model->data(status_model->index(ui->status_comboBox->currentIndex(), status_model->fieldIndex("id"))).toInt())
                        )){
            qDebug() << "Can not save item into database. Sql error: " << query->lastError().text();
            if(logger) logger->writeLog(Logger::Error, Logger::Items,
                                         tr("Can not save item.\n%1").arg(query->lastError().text())
                                        );
            return false;
        }
        int item_id = query->lastInsertId().toInt();
        //setItemId(item_id);
        if(!query->exec(QString("UPDATE items SET `number`='%1',`manufacturer`='%2',`model`='%3',"
                                "`serialno`='%4',`IP`='%5',`note`='%6' WHERE `id`='%7'")
                        .arg(ui->number_lineEdit->text())
                        .arg(ui->manufacturer_lineEdit->text())
                        .arg(ui->model_lineEdit->text())
                        .arg(ui->serialno_lineEdit->text())
                        .arg(ui->ipaddress_lineEdit->text())
                        .arg(ui->note_textEdit->toPlainText())
                        .arg(item_id)
                        )){
            qDebug() << "Can not fill all item fields. Sql error: " << query->lastError().text();
            if(logger) logger->writeLog(Logger::Error, Logger::Items,
                                         tr("Can not fill all item fields.\n%1").arg(query->lastError().text())
                                        );
        }
        int old_operator_id = ui->operator_comboBox->findData("N/A", Qt::EditRole);
        int old_location_id = ui->location_comboBox->findData("N/A", Qt::EditRole);
        if(old_operator_id == -1) old_operator_id = new_operator_id;
        else old_operator_id = operators_model->data(operators_model->index(old_operator_id, operators_model->fieldIndex("id"))).toInt();
        if(old_location_id == -1) old_location_id = new_location_id;
        else old_location_id = locations_model->data(locations_model->index(old_location_id, locations_model->fieldIndex("id"))).toInt();
        if(!query->exec(QString("SELECT * FROM items WHERE `id`=%1").arg(item_id))){
            if(logger) logger->writeLog(Logger::Error, Logger::Items,
                                         tr("Can not load item information.\n%1").arg(query->lastError().text())
                                        );
        }
        else{
            if(logger) logger->writeLog(Logger::Add, Logger::Items,
                                        tr("New item was defined\n%1").arg(logger->infoLog(query))
                                        );
        }
        if(!query->exec(QString("INSERT INTO allocations (`date`,`item_id`,`from_operator_id`,`from_location_id`,"
                                "`to_operator_id`,`to_location_id`,`note`) VALUES ('%1', '%2', '%3', '%4', '%5', '%6', '%7')")
                        .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm"))
                        .arg(item_id)
                        .arg(old_operator_id)
                        .arg(old_location_id)
                        .arg(new_operator_id)
                        .arg(new_location_id)
                        .arg(tr("Automatic allocation. New item was defined"))
                        )){
            if(logger) logger->writeLog(Logger::Error, Logger::Items,
                                         tr("Can not save item allocation into database.\n%1").arg(query->lastError().text())
                                        );
        }
        if(!query->exec(QString("SELECT * FROM allocations WHERE `id`=%1").arg(query->lastInsertId().toInt()))){
            if(logger) logger->writeLog(Logger::Error, Logger::Items,
                                         tr("Can not load allocation information.\n%1").arg(query->lastError().text())
                                        );
        }
        else{
            if(logger) logger->writeLog(Logger::Add, Logger::Allocations,
                                        tr("New automatic allocation was performed%1").arg(logger->infoLog(query))
                                        );
        }
        return true;
    }
    else{
        bool new_allocation = false;
        QString itemlog;
        if(new_operator_id != getOperatorId() || new_location_id != getLocationId()){
            int ret = QMessageBox::question(0, tr("Items"),
                                            tr("You have made a change in this item.\n"
                                               "Must be performed new, automatic allocation.\n"
                                               "Do you want to continue?\n"
                                               "This could not be restored!"),
                                                QMessageBox::Ok | QMessageBox::Cancel);
            if(ret == QMessageBox::Cancel) return false;
            new_allocation = true;
        }
        QSqlQuery* query = new QSqlQuery(QSqlDatabase::database());
        if(!query->exec(QString("SELECT * FROM items WHERE `id`=%1").arg(getItemId()))){
            qDebug() << "Can not load item info. Sql error: " << query->lastError().text();
            if(logger) logger->writeLog(Logger::Error, Logger::Items,
                                         tr("Can not load item info for logging.\n%1").arg(query->lastError().text())
                                        );
        }
        else{
            if(logger) itemlog = logger->infoLog(query);
        }
        if(!query->exec(QString("UPDATE items SET `number`='%1',`manufacturer`='%2',`model`='%3',"
                                "`serialno`='%4',`IP`='%5',`note`='%6',"
                                "`status_id`='%7', `type_id`='%8' WHERE `id`='%9'")
                        .arg(ui->number_lineEdit->text())
                        .arg(ui->manufacturer_lineEdit->text())
                        .arg(ui->model_lineEdit->text())
                        .arg(ui->serialno_lineEdit->text())
                        .arg(ui->ipaddress_lineEdit->text())
                        .arg(ui->note_textEdit->toPlainText())
                        .arg(status_model->data(status_model->index(ui->status_comboBox->currentIndex(), status_model->fieldIndex("id"))).toInt())
                        .arg(types_model->data(types_model->index(ui->type_comboBox->currentIndex(), types_model->fieldIndex("id"))).toInt())
                        .arg(getItemId())
                        )){
            qDebug() << "Can not save item changes. Sql error: " << query->lastError().text();
            if(logger) logger->writeLog(Logger::Error, Logger::Items,
                                         tr("Can not save item changes.\n%1").arg(query->lastError().text())
                                        );
            return false;
        }
        if(!query->exec(QString("UPDATE items SET `location_id`='%1',`operator_id`='%2'"
                                " WHERE `id`='%3'")
                        .arg(new_location_id)
                        .arg(new_operator_id)
                        .arg(getItemId())
                        )){
            qDebug() << "Can not save item changes. Sql error: " << query->lastError().text();
            if(logger) logger->writeLog(Logger::Error, Logger::Items,
                                         tr("Can not save item changes.\n%1").arg(query->lastError().text())
                                        );
            return false;
        }
        if(new_allocation){
            if(!query->exec(QString("INSERT INTO allocations (`date`,`item_id`,`from_operator_id`,`from_location_id`,"
                                    "`to_operator_id`,`to_location_id`,`note`) VALUES ('%1', '%2', '%3', '%4', '%5', '%6', '%7')")
                            .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm"))
                            .arg(getItemId())
                            .arg(getOperatorId())
                            .arg(getLocationId())
                            .arg(new_operator_id)
                            .arg(new_location_id)
                            .arg(tr("Automatic allocation. This item was edited!"))
                            )){
                if(logger) logger->writeLog(Logger::Error, Logger::Items,
                                             tr("Can not save item allocation into database.\n%1").arg(query->lastError().text())
                                            );
                return false;
            }
        }
        if(logger) logger->writeLog(Logger::Edit, Logger::Items,
                                    tr("Save edited item: %1").arg(itemlog));
        return true;
    }
    return false;
}

void Items::loadData()
{
    if(!types_model) types_model = new QSqlTableModel(ui->type_comboBox);
    types_model->setTable("item_types");
    types_model->select();

    ui->type_comboBox->setModel(types_model);
    ui->type_comboBox->setModelColumn(types_model->fieldIndex("type"));
    ui->type_comboBox->setCurrentIndex(0);

    if(!operators_model) operators_model = new QSqlTableModel(ui->operator_comboBox);
    operators_model->setTable("operators");
    operators_model->select();

    ui->operator_comboBox->setModel(operators_model);
    ui->operator_comboBox->setModelColumn(operators_model->fieldIndex("name"));
    ui->operator_comboBox->setCurrentIndex(ui->operator_comboBox->findData("N/A", Qt::EditRole));

    if(!locations_model) locations_model = new QSqlTableModel(ui->location_comboBox);
    locations_model->setTable("locations");
    locations_model->select();

    ui->location_comboBox->setModel(locations_model);
    ui->location_comboBox->setModelColumn(locations_model->fieldIndex("location"));
    ui->location_comboBox->setCurrentIndex(ui->location_comboBox->findData("N/A", Qt::EditRole));

    if(!status_model) status_model = new QSqlTableModel(ui->status_comboBox);
    status_model->setTable("item_status");
    status_model->select();

    ui->status_comboBox->setModel(status_model);
    ui->status_comboBox->setModelColumn(status_model->fieldIndex("status"));
    ui->status_comboBox->setCurrentIndex(0);
}

void Items::setEditMode(const bool edit)
{
    this->edit_mode = edit;

    //ui->operator_comboBox->setDisabled(edit);
    //ui->location_comboBox->setDisabled(edit);

    setWindowTitle(tr("Items (%1 item)").arg(edit ? tr("edit") : tr("new")));
}

void Items::loadItem(const int item_id)
{
    QSqlQuery* query = new QSqlQuery(QSqlDatabase::database());
    if(!query->exec(QString("SELECT * FROM items WHERE `id`=%1")
                        .arg(item_id))){
        qDebug() << "Can not get item data from database. Sql error: " << query->lastError().text();
        return;
    }
    if(!query->first()){
        qDebug() << "No item with id: " << item_id;
        return;
    }
    QSqlRecord record = query->record();

    setItemId(item_id);
    setOperatorId(record.value("operator_id").toInt());
    setOperatorFilter(getOperatorId());
    setLocationId(record.value("location_id").toInt());
    setLocationFilter(getLocationId());
    setStatusFilter(record.value("status_id").toInt());
    setTypeFilter(record.value("type_id").toInt());
    ui->number_lineEdit->setText(record.value("number").toString());
    ui->ipaddress_lineEdit->setText(record.value("IP").toString());
    ui->manufacturer_lineEdit->setText(record.value("manufacturer").toString());
    ui->model_lineEdit->setText(record.value("model").toString());
    ui->serialno_lineEdit->setText(record.value("serialno").toString());
    ui->note_textEdit->setText(record.value("note").toString());

    this->fillProperties(record.value("type_id").toInt(), item_id);
    this->fillAttachments(item_id);
}

void Items::setLocationFilter(const int location_id)
{
    if(!locations_model || !location_id) return;
    QModelIndexList Locations = locations_model->match(locations_model->index( 0, locations_model->fieldIndex("id")), Qt::DisplayRole, location_id, 1, Qt::MatchExactly);
    ui->location_comboBox->setCurrentIndex(Locations[0].row());
}

void Items::setOperatorFilter(const int operator_id)
{
    if(!operators_model || !operator_id) return;
    QModelIndexList Operators = operators_model->match(operators_model->index( 0, operators_model->fieldIndex("id")), Qt::DisplayRole, operator_id,  1, Qt::MatchExactly);
    ui->operator_comboBox->setCurrentIndex(Operators[0].row());
}

void Items::setTypeFilter(const int type_id)
{
    if(!types_model || !type_id) return;
    QModelIndexList Types = types_model->match(types_model->index( 0, types_model->fieldIndex("id")), Qt::DisplayRole, type_id,  1, Qt::MatchExactly);
    ui->type_comboBox->setCurrentIndex(Types[0].row());
}

void Items::setStatusFilter(const int status_id)
{
    if(!status_model || !status_id) return;
    QModelIndexList Status = status_model->match(status_model->index( 0, status_model->fieldIndex("id")), Qt::DisplayRole, status_id,  1, Qt::MatchExactly);
    ui->status_comboBox->setCurrentIndex(Status[0].row());
}

void Items::setLineEditCompleter(QLineEdit *lineEdit, const QString &field, const QString &table)
{
    QSqlQuery* query = new QSqlQuery();
    if(!query->exec(QString("SELECT `%1` FROM `%2` GROUP BY `%1`")
                    .arg(field)
                    .arg(table)
                    )){
        if(logger) logger->writeLog(Logger::Error, Logger::Other,
                                    tr("Sql error: \n%1").arg(query->lastError().text())
                                    );
        return;
    }
    QStringList wordList;
    while(query->next()){
        wordList << query->value(0).toString();
    }
    QCompleter *completer = new QCompleter(wordList, this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    lineEdit->setCompleter(completer);
}

bool Items::checkNumber() const
{
    QString number = ui->number_lineEdit->text();
    if(number.isEmpty() || number == "N/A" || number == "N\\A")
        return true;
    QSqlQuery* query = new QSqlQuery();
    QString q = QString("SELECT * FROM items WHERE `number` LIKE '%1'").arg(number);
    if(isEditMode()){
        q.append(QString(" AND `id` NOT LIKE '%1'").arg(getItemId()));
    }
    if(!query->exec(q)){
        if(logger) logger->writeLog(Logger::Error, Logger::Items, tr("Sql error:\n%1")
                                    .arg(query->lastError().text())
                                    );
        return false;
    }
    if(query->first()){
        int ret = QMessageBox::question(0, tr("Items"),
                                    tr("There is already a item with this number.\n"
                                       "Do you want to continue?\n"),
                                        QMessageBox::Ok | QMessageBox::Cancel);
        if(ret == QMessageBox::Cancel){
            ui->number_lineEdit->selectAll();
            ui->number_lineEdit->setFocus();
            return false;
        }
    }
    return true;
}

void Items::fillProperties(const int type_id, const int item_id)
{
    if(type_id <=0 || item_id < 0) return;
    QSqlQuery* query = new QSqlQuery();
    QString q = QString("SELECT id, property FROM properties "
                        "WHERE type_id = %1")
            .arg(type_id);
    if(!query->exec(q)){
        if(logger) logger->writeLog(Logger::Error, Logger::Properties, tr("Sql error:\n%1")
                                    .arg(query->lastError().text())
                                    );
        return;
    }
    QStandardItemModel* model = new QStandardItemModel(0,2,ui->properties_treeView);
    model->setHeaderData(0, Qt::Horizontal, tr("Name"));
    model->setHeaderData(1, Qt::Horizontal, tr("Value"));
    while (query->next()){
        QSqlQuery* sub = new QSqlQuery(QString("SELECT id, value FROM item_properties "
                                               "WHERE item_id = %1 AND property_id = %2")
                                       .arg(item_id)
                                       .arg(query->value(0).toInt())
                                       );
        QString value;
        int id = 0;
        if(sub->exec()){
            if(sub->first()){
                id = sub->value(0).toInt();
                value = sub->value(1).toString();
            }
        }
        QStandardItem* item0 = new QStandardItem(query->value(1).toString());
        QStandardItem* item1 = new QStandardItem(value);

        item0->setEditable(false);
        item0->setData(query->value(0).toInt());
        item1->setData(id);
        /* fill item editor completer */
        QStringList list;
        if(sub->exec(QString("SELECT `value` FROM item_properties "
                             "WHERE `property_id` = %1")
                     .arg(query->value(0).toInt())
                     )){
            while(sub->next()){
                list << sub->value(0).toString();
            }
            list.removeDuplicates();
        }
        item1->setData(list, Qt::UserRole);

        model->setItem(model->rowCount(), 0, item0);
        model->setItem(model->rowCount()-1, 1, item1);
    }

    ui->properties_treeView->setModel(model);
    ui->properties_treeView->setItemDelegate(new PropertyItemDelegate(ui->properties_treeView));
}

void Items::saveItemProperties()
{
    QStandardItemModel* model = qobject_cast<QStandardItemModel *>(ui->properties_treeView->model());
    if(!model) return;
    QSqlQuery* query = new QSqlQuery();

    if(!query->exec(QString("DELETE FROM item_properties WHERE `item_id` = '%1'")
                   .arg(getItemId())
                   )){
        if(logger) logger->writeLog(Logger::Error, Logger::Properties, tr("Sql error:\n%1")
                                    .arg(query->lastError().text())
                                    );
    }

    for (int i = 0; i < model->rowCount(); ++i) {
         QStandardItem *item0 = model->item(i,0);
         QStandardItem *item1 = model->item(i,1);
         if(!item1->data(Qt::EditRole).toString().isEmpty()){
             if(!query->exec(QString("INSERT INTO item_properties (`property_id`,`item_id`,`value`) "
                                     "VALUES ('%1','%2','%3')")
                             .arg(item0->data().toInt())
                             .arg(this->getItemId())
                             .arg(item1->data(Qt::EditRole).toString())
                             ))
             {
                 if(logger) logger->writeLog(Logger::Error, Logger::Properties, tr("Sql error:\n%1")
                                             .arg(query->lastError().text())
                                             );
             }
         }
    }
}

void Items::on_type_comboBox_currentIndexChanged(int /*index*/)
{
    this->fillProperties(ui->type_comboBox->itemModelId());
}

void Items::fillAttachments(const int item_id)
{
    if(item_id <= 0) return;
    QSqlQuery* query = new QSqlQuery();
    QString q = QString("SELECT `id`, `filename` FROM attachments "
                        "WHERE `tablename`='items' AND `item_id`='%1'")
            .arg(item_id);
    if(!query->exec(q)){
        if(logger) logger->writeLog(Logger::Error, Logger::Properties, tr("Sql error:\n%1")
                                    .arg(query->lastError().text())
                                    );
        return;
    }
    MyStandardItemModel* model = qobject_cast<MyStandardItemModel *>(ui->attachments_listView->model());
    model->setHeaderData(0, Qt::Horizontal, tr("File name"));
    while (query->next()){
        QStandardItem* item = new QStandardItem(query->value(1).toString());

        item->setEditable(false);
        item->setData(query->value(0).toInt(), Qt::UserRole+1);
        item->setData(item_id, Qt::UserRole+2);

        QFileInfo fi(qApp->property("data_path").toString());
        QDir dir(fi.absoluteDir());
        dir.cd("attachments");

        item->setData(dir.absoluteFilePath(query->value(1).toString()), Qt::UserRole+3);//path
        item->setToolTip(item->data(Qt::UserRole+3).toString());

        model->setItem(model->rowCount(), 0, item);
    }
}

void Items::saveItemAttachments()
{
    if(!this->getItemId()) return;

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

    QSqlQuery* query = new QSqlQuery();
    MyStandardItemModel* model = qobject_cast<MyStandardItemModel *>(ui->attachments_listView->model());
    if(model){
        for(int i=0; i<model->rowCount(); i++){
            if(model->item(i)->data().toInt() == 0){    //new file
                QFile source(model->item(i)->data(Qt::UserRole+3).toString());
                fi.setFile(source);
                QString q = QString("SELECT COUNT(`id`) FROM attachments "
                                    "WHERE `filename` LIKE '%1'")
                        .arg(model->item(i)->data(Qt::EditRole).toString());
                if(query->exec(q)){
                    query->first();
                    if(query->value(0).toInt() == 0){//there is no file with that name
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
                        }
                    }
                    if(!query->exec(QString("INSERT INTO attachments (`item_id`, `tablename`, `filename`) VALUES ('%1', '%2', '%3')")
                                        .arg(this->getItemId())
                                        .arg("items")
                                        .arg(fi.fileName()))
                       ){
                        if(logger) logger->writeLog(Logger::Error, Logger::Attachments,
                            tr("Sql error:\n%1").arg(query->lastError().text())
                            );
                     }
                     else{
                            if(logger) logger->writeLog(Logger::Add, Logger::Attachments,
                                                        tr("Attacment: %1 has been added to:\n%2")
                                                        .arg(fi.fileName())
                                                        .arg(tr("Item with id: %1").arg(this->getItemId()))
                                                        );
                     }
                }
                else{
                    if(logger) logger->writeLog(Logger::Error, Logger::Attachments, tr("Sql error:\n%1")
                                                .arg(query->lastError().text())
                                                );
                }
            }
        }
    }
    QList<QString> del = ui->attachments_listView->pendedAttachments();
    for (int i = 0; i < del.size(); ++i) {
        QString q = QString("DELETE FROM attachments "
                            "WHERE `filename` LIKE '%1' AND `item_id`='%2'")
                .arg(del.at(i))
                .arg(this->getItemId())
                ;
        if(!query->exec(q)){
            if(logger) logger->writeLog(Logger::Error, Logger::Attachments, tr("Sql error:\n%1")
                                        .arg(query->lastError().text())
                                        );
        }
        q = QString("SELECT COUNT(`id`) FROM attachments "
                            "WHERE `filename` LIKE '%1'").arg(del.at(i));
        if(!query->exec(q)){
            if(logger) logger->writeLog(Logger::Error, Logger::Attachments, tr("Sql error:\n%1")
                                        .arg(query->lastError().text())
                                        );
        }
        else{
            query->first();
            if(query->value(0).toInt() == 0){//no other uses of the file
                QFileInfo fi_del(dir, del.at(i));
                if(!QFile::remove(fi_del.absoluteFilePath())){
                    if(logger) logger->writeLog(Logger::Error, Logger::Attachments,
                                                tr("Can not remove file!"));
                }
                else{
                    if(logger) logger->writeLog(Logger::Add, Logger::Attachments,
                                                tr("File was successfully removed:\n%1").arg(dir.absoluteFilePath(fi_del.fileName()))
                                                );
                }
            }
        }
    }

}

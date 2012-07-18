#include "inventorytableview.h"

#include <QtGui>
#include <QtSql>

#include "rights.h"
#include "items.h"
#include "scrap.h"
#include "allocations.h"
#include "logger.h"
#include "checkdelegate.h"

MySqlRelationalTableModel::MySqlRelationalTableModel(QObject *parent) :
    QSqlRelationalTableModel(parent)
{
}

QStringList MySqlRelationalTableModel::mimeTypes() const
{
    QStringList types;
    types << "text/uri-list";
    return types;
}

InventoryTableView::InventoryTableView(QWidget *parent):
    QTableView(parent)
{
    logger = 0;

    setAcceptDrops(true);
    setDropIndicatorShown(true);

    add_attachment = new QAction(QIcon(":/Icons/icons/Attach.png"), tr("Add attachment"), this);
    open_attachment = new QAction(QIcon(":/Icons/icons/Document.png"), tr("Open"), this);
    delete_attachment = new QAction(QIcon(":/Icons/icons/Trash.png"), tr("Delete"), this);

    open_rights = new QAction(QIcon(":/Icons/icons/Key.png"), tr("Edit"), this);

    add_item = new QAction(QIcon(":/Icons/icons/Computer.png"), tr("Add"), this);
    edit_item = new QAction(QIcon(":/Icons/icons/Write2.png"), tr("Edit"), this);
    copy_item = new QAction(QIcon(":/Icons/icons/Applications.png"), tr("Copy"), this);
    scrap_item = new QAction(QIcon(":/Icons/icons/Trash.png"), tr("Scrapping"), this);
    delete_item = new QAction(QIcon(":/Icons/icons/Trash.png"), tr("Delete"), this);
    allocation = new QAction(QIcon(":/Icons/icons/Puzzle.png"), tr("Allocation"), this);

    find_item = new QAction(QIcon(":/Icons/icons/Zoom In.png"), tr("Find item"), this);
    find_allocation = new QAction(QIcon(":/Icons/icons/Zoom In.png"), tr("Find allocation"), this);

    add_property = new QAction(QIcon(":/Icons/icons/Tag.png"), tr("Add property"), this);
    edit_property = new QAction(QIcon(":/Icons/icons/Tool.png"), tr("Edit property"), this);
    delete_property = new QAction(QIcon(":/Icons/icons/Trash.png"), tr("Delete property"), this);

    mark_row = new QAction(QIcon(":/Icons/icons/Ok.png"), tr("Mark row"), this);
    unmark_row = new QAction(QIcon(":/Icons/icons/Cancel.png"), tr("Unmark row"), this);
    marking_reverse = new QAction(QIcon(":/Icons/icons/Refresh.png"), tr("Marking reverse"), this);
    hide_marked = new QAction(QIcon(":/Icons/icons/Dots Down.png"), tr("Hide marked"), this);
    hide_marked->setCheckable(true);
    hide_unmarked = new QAction(QIcon(":/Icons/icons/Dots Up.png"), tr("Hide unmarked"), this);
    hide_unmarked->setCheckable(true);

    print = new QAction(QIcon(":/Icons/icons/Printer.png"), tr("Print"), this);

    /*** status tips ***/

    add_attachment->setStatusTip(tr("Add attachment to the selected object"));
    open_attachment->setStatusTip(tr("Opens attachment with its associated program"));
    delete_attachment->setStatusTip(tr("Deletes the attachment"));

    open_rights->setStatusTip(tr("Edit the selected user rights"));
    add_item->setStatusTip(tr("Add a item"));
    edit_item->setStatusTip(tr("Edit current item"));
    copy_item->setStatusTip(tr("Add a item based on the selected item"));
    scrap_item->setStatusTip(tr("Scrapping of the selected item"));
    delete_item->setStatusTip(tr("Delete current item"));
    allocation->setStatusTip(tr("Allocation of the selected item to a new location / operator"));
    find_item->setStatusTip(tr("Find the item associated with selected row"));
    find_allocation->setStatusTip(tr("Find the allocation associated with selected row"));
    mark_row->setStatusTip(tr("Mark the selected row"));
    unmark_row->setStatusTip(tr("Unmark the selected row"));
    marking_reverse->setStatusTip(tr("Reversed the current marking"));
    hide_marked->setStatusTip(tr("Hide the marked rows"));
    hide_unmarked->setStatusTip(tr("Hide the unmarked rows"));
    print->setStatusTip(tr("Print current table"));

    add_property->setStatusTip(tr("Add property to the selected item type"));
    edit_property->setStatusTip(tr("Edit current property"));
    delete_property->setStatusTip(tr("Delete current property"));



    connect(add_attachment, SIGNAL(triggered()), this, SLOT(addAttachment()));
    connect(open_attachment, SIGNAL(triggered()), this, SLOT(openAttachment()));    
    connect(delete_attachment, SIGNAL(triggered()), this, SLOT(deleteAttachment()));

    connect(add_property, SIGNAL(triggered()), this, SLOT(addProperty()));
    connect(edit_property, SIGNAL(triggered()), this, SLOT(editProperty()));
    connect(delete_property, SIGNAL(triggered()), this, SLOT(deleteProperty()));

    connect(open_rights, SIGNAL(triggered()), this, SLOT(openRights()));

    connect(add_item, SIGNAL(triggered()), this, SLOT(addItem()));
    connect(edit_item, SIGNAL(triggered()), this, SLOT(editItem()));
    connect(copy_item, SIGNAL(triggered()), this, SLOT(copyItem()));
    connect(scrap_item, SIGNAL(triggered()), this, SLOT(scrapItem()));
    connect(delete_item, SIGNAL(triggered()), this, SLOT(deleteItem()));
    connect(allocation, SIGNAL(triggered()), this, SLOT(allocationItem()));

    connect(find_item, SIGNAL(triggered()), this, SLOT(findItem()));
    connect(find_allocation, SIGNAL(triggered()), this, SLOT(findAllocation()));

    connect(mark_row, SIGNAL(triggered()), this, SLOT(markRow()));
    connect(unmark_row, SIGNAL(triggered()), this, SLOT(unmarkRow()));
    connect(marking_reverse, SIGNAL(triggered()), this, SLOT(markingReverse()));
    connect(hide_marked, SIGNAL(triggered(bool)), this, SLOT(hideMarked(bool)));
    connect(hide_unmarked, SIGNAL(triggered(bool)), this, SLOT(hideUnmarked(bool)));

    connect(print, SIGNAL(triggered()), this, SLOT(printPreviewTable()));

    connect(this, SIGNAL(activated(QModelIndex)), this, SLOT(activated(QModelIndex)));
}

InventoryTableView::~InventoryTableView()
{
}

void InventoryTableView::contextMenuEvent(QContextMenuEvent *event)
{
    if(this->model()){
        QSqlRelationalTableModel *model = qobject_cast<QSqlRelationalTableModel *>(this->model());
        QModelIndex index = this->selectionModel()->currentIndex();
        QMenu menu;
        if(index.isValid()){
            if(qApp->property("marking_rows").toBool()){
                if(qobject_cast<CheckDelegate *>(itemDelegateForRow(index.row())))
                    menu.addAction(unmark_row);
                else
                    menu.addAction(mark_row);
                menu.addAction(marking_reverse);
                menu.addAction(hide_marked);
                menu.addAction(hide_unmarked);
                menu.addSeparator();
            }

            /*
            QStringList tablesWithAttachment;
            tablesWithAttachment << "items" << "locations" << "allocations" << "item_types" << "item_status" << "operators" << "users";
            if(tablesWithAttachment.contains(model->tableName())){
                menu.addSeparator();
                menu.addAction(add_attachment);
            }
            */
        }
        if(model->tableName() == "attachments"){
            menu.addAction(add_attachment);
            if(index.isValid()){
                menu.addSeparator();
                menu.addAction(open_attachment);
                menu.addAction(delete_attachment);
            }
        }
        else if(model->tableName() == "users_rights"){
            if(index.isValid()){
                menu.addAction(open_rights);
            }
        }
        else if(model->tableName() == "items"){
            menu.addAction(add_item);
            if(index.isValid()){
                menu.addAction(edit_item);
                menu.addAction(copy_item);
                menu.addAction(allocation);
                menu.addSeparator();
                menu.addAction(find_allocation);
                menu.addAction(find_item);
                menu.addSeparator();
                menu.addAction(scrap_item);
                menu.addAction(delete_item);
            }
        }
        else if(model->tableName() == "allocations"){
            if(index.isValid()){                
                menu.addAction(allocation);
                menu.addAction(find_item);
            }
        }
        else if(model->tableName() == "temp_reference_items" || model->tableName() == "temp_reference_items_properties"){
            if(index.isValid()){
                menu.addAction(find_item);
                menu.addAction(find_allocation);
            }
        }
        else if(model->tableName() == "properties"){
            menu.addAction(add_property);
            if(index.isValid()){
                menu.addAction(edit_property);
                menu.addAction(delete_property);
            }
        }
        menu.addSeparator();
        menu.addAction(print);
        menu.exec(event->globalPos());
    }    
}

void InventoryTableView::openAttachment()
{
    if(!this->model()) return;
    QSqlRelationalTableModel *model = qobject_cast<QSqlRelationalTableModel *>(this->model());
    QModelIndex index = this->selectionModel()->currentIndex();
    QSqlRecord record = model->record(index.row());
    if(!record.isEmpty()){
        //qDebug() << "Attacment ID: " << record.value("id").toInt();
        //qDebug() << "Attacment filename: " << record.value("filename").toString();
        QFileInfo fi(qApp->property("data_path").toString());
        QDir dir(fi.absoluteDir());
        if(!dir.exists()){
            qDebug() << "Can not find data directory!";
            return;
        }
        if(!dir.cd("attachments")){
            qDebug() << "Error! Can not find attachments directory. The operation is interrupted.";
            return;
        }
        QString filePath = dir.absoluteFilePath(record.value("filename").toString());
        QUrl url;
        if (filePath.startsWith("\\\\") || filePath.startsWith("//"))
        url.setUrl(QDir::toNativeSeparators(filePath));
        else
        url = QUrl::fromLocalFile(filePath);
        if(!QDesktopServices::openUrl(url)){
            if(logger) logger->writeLog(Logger::Error, Logger::Attachments,
                                        tr("Error with document opening. The url path:\n%1")
                                        .arg(dir.absoluteFilePath(record.value("filename").toString()))
                                        );
        }
        else{
            if(logger) logger->writeLog(Logger::View, Logger::Attachments, tr("Open attachment: %1")
                                        .arg(filePath));
        }
    }
}

void InventoryTableView::addAttachment()
{
    if(!this->checkUserRights(16)) return;
    emit add_attachment_();
}

void InventoryTableView::deleteAttachment()
{
    if(!this->checkUserRights(17)) return;
    if(!this->model()) return;
    QSqlRelationalTableModel *model = qobject_cast<QSqlRelationalTableModel *>(this->model());
    QModelIndex index = this->selectionModel()->currentIndex();
    QSqlRecord record = model->record(index.row());
    if(!record.isEmpty()){
        QString filename = record.value("filename").toString();
        int ret = QMessageBox::question(this, tr("Attachments"),
                tr("Do you want to delete %1?\nThis could not be restored!")
                                    .arg(filename),
                                    QMessageBox::Ok | QMessageBox::Cancel);
        if(ret == QMessageBox::Cancel) return;
        QString info;
        if(logger) info = logger->infoLog(record);
        QSqlQuery* query = new QSqlQuery(QSqlDatabase::database());
        if(!query->exec(QString("DELETE FROM attachments WHERE `id`=%1")
                            .arg(record.value("id").toInt()))
            ){
            if(logger) logger->writeLog(Logger::Error, Logger::Attachments,
                                        tr("Can not remove from database. Sql error:\n%1").arg(query->lastError().text())
                                        );
        }
        else{
            if(logger) logger->writeLog(Logger::Delete, Logger::Attachments,
                                        tr("Delete attachment info:\n%1").arg(info));
        }
        if(!query->exec(QString("SELECT `id` FROM attachments WHERE `filename` LIKE '%1'")
                        .arg(filename))){
            if(logger) logger->writeLog(Logger::Error, Logger::Attachments,
                                        tr("Sql error:\n%1").arg(query->lastError().text())
                                        );
        }
        else if(query->first()){
            qDebug() << "There is other items that use this resource.\nThe operation is interrupted.";
            emit tableUpdate("attachments");
            return;
        }
        else emit tableUpdate("attachments");

        QFileInfo fi(qApp->property("data_path").toString());
        QDir dir(fi.absoluteDir());
        if(!dir.exists()){
            qDebug() << "Can not find data directory!";
            if(logger) logger->writeLog(Logger::Error, Logger::Attachments,
                                        tr("Can not find data directory!")
                                        );
            return;
        }
        if(!dir.cd("attachments")){
            if(logger) logger->writeLog(Logger::Error, Logger::Attachments,
                                        tr("Can not find attachments directory."));
            return;
        }
        if(!QFile::remove(dir.absoluteFilePath(filename))){
            if(logger) logger->writeLog(Logger::Error, Logger::Attachments,
                                        tr("Can not delete file:\n%1").arg(filename)
                                        );

        }
        else{
            qDebug() << "File was successfully deleted :)";
            if(logger) logger->writeLog(Logger::Delete, Logger::Attachments,
                                        tr("Deleted file:\n%1").arg(dir.absoluteFilePath(filename))
                                        );
        }
    }
}

void InventoryTableView::deleteItem()
{
    if(!this->checkUserRights(5)) return;
    if(!this->model()) return;
    QSqlRelationalTableModel *model = qobject_cast<QSqlRelationalTableModel *>(this->model());
    QModelIndex index = this->selectionModel()->currentIndex();
    QSqlRecord record = model->record(index.row());
    if(!record.isEmpty()){
        int ret = QMessageBox::question(this, tr("Items"),
                tr("Do you want to delete item %1?\nThis could not be restored!")
                                    .arg(record.value("number").toString()),
                                    QMessageBox::Ok | QMessageBox::Cancel);
        if(ret == QMessageBox::Cancel) return;
        QString info;
        if(logger) info = logger->infoLog(record);
        QSqlQuery* query = new QSqlQuery(QSqlDatabase::database());
        if(!query->exec(QString("DELETE FROM items WHERE `id`=%1")
                            .arg(record.value("id").toInt()))
            ){
            if(logger) logger->writeLog(Logger::Error, Logger::Items,
                                        tr("Can not remove item from database.\n%1").arg(query->lastError().text())
                                        );
        }
        else emit tableUpdate("items");
        if(logger) logger->writeLog(Logger::Delete, Logger::Items,
                                    tr("Deleted item:\n%1").arg(info)
                                    );
    }
}

void InventoryTableView::addItem()
{
    if(!this->checkUserRights(4)) return;
    emit add_item_();
}

void InventoryTableView::activated(const QModelIndex& index)
{
    if(!this->model()) return;
    QSqlRelationalTableModel *model = qobject_cast<QSqlRelationalTableModel *>(this->model());
    if(model->tableName() == "attachments"){
        this->openAttachment();
        return;
    }
    if(model->tableName() == "users_rights"){
        this->openRights();
        return;
    }
    if(model->tableName() == "properties"){
        this->editProperty();
        return;
    }
    if(!index.isValid()) return;
    emit activatedItem(model->index(index.row(), model->fieldIndex("id")).data().toInt(), model->tableName());
}

void InventoryTableView::openRights()
{
    if(!this->checkUserRights(1)) return;
    if(!this->model()) return;
    QSqlRelationalTableModel *model = qobject_cast<QSqlRelationalTableModel *>(this->model());
    QModelIndex index = this->selectionModel()->currentIndex();
    QSqlRecord record = model->record(index.row());
    Rights* dialog = new Rights(this, Qt::MSWindowsFixedSizeDialogHint);
    dialog->setId(record.value("id").toInt());
    dialog->loadRights();
    if(dialog->exec() == QDialog::Accepted){
        emit tableUpdate("users_rights");
        if(logger) logger->writeLog(Logger::Edit, Logger::Users,
                                    tr("Edit user rights. Old data:\n%1").arg(logger->infoLog(record))
                                    );
    }
    delete dialog;
}

bool InventoryTableView::checkUserRights(const int idx, const bool show_message) const
{
    QBitArray rights(qApp->property("user_rights").toBitArray());
    if(!rights[idx]){
        if(show_message){
        QMessageBox msgBox;
            msgBox.setText(tr("You are not authorized to perform this operation!\nPlease contact your system administrator."));
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.exec();
        }
        if(logger) logger->writeLog(Logger::Info, Logger::Other, QString("Attempt to commit the procedure %1 without any rights")
                                    .arg(idx)
                                    );
        return false;
    }
    else {
        return true;
    }
}

void InventoryTableView::editItem()
{
    if(!this->checkUserRights(4)) return;
    if(!this->model()) return;
    QSqlRelationalTableModel *model = qobject_cast<QSqlRelationalTableModel *>(this->model());
    QModelIndex index = this->selectionModel()->currentIndex();
    QSqlRecord record = model->record(index.row());
    if(!record.isEmpty()){
        Items* items_dialog = new Items(this);
        if(logger) items_dialog->setLogger(logger);
        items_dialog->setEditMode(true);
        items_dialog->loadItem(record.value("id").toInt());
        if(items_dialog->exec() == QDialog::Accepted){
            emit tableUpdate("Items");
            if(logger) logger->writeLog(Logger::Edit, Logger::Items,
                                        tr("Edit item. Old data:\n%1").arg(logger->infoLog(record))
                                        );
        }
        delete items_dialog;
    }
}

void InventoryTableView::allocationItem()
{
    if(!this->checkUserRights(26)) return;
    //if(!this->checkUserRights(4)) return;
    if(!this->model()) return;
    QSqlRelationalTableModel *model = qobject_cast<QSqlRelationalTableModel *>(this->model());
    QModelIndex index = this->selectionModel()->currentIndex();
    QSqlRecord record = model->record(index.row());
    if(!record.isEmpty()){
        Allocations* allocations_dialog = new Allocations(this);
        if(logger) allocations_dialog->setLogger(logger);
        allocations_dialog->setEditMode(false);
        if(model->tableName() == "items"){
            allocations_dialog->setItemFilter(record.value("id").toInt());
        }
        if(model->tableName() == "allocations"){
            QSqlQuery* query = new QSqlQuery(QSqlDatabase::database());
            if(!query->exec(QString("SELECT `item_id` FROM allocations WHERE `id`=%1")
                                .arg(record.value("id").toInt()))
                ){
                if(logger) logger->writeLog(Logger::Error, Logger::Allocations,
                                            tr("Can not get item id from database.\n%1").arg(query->lastError().text())
                                            );
            }
            else{
                query->first();
                allocations_dialog->setItemFilter(query->value(0).toInt());
            }
        }
        if(allocations_dialog->exec() == QDialog::Accepted){
            emit tableUpdate("Allocations");
            if(logger) logger->writeLog(Logger::Edit, Logger::Allocations,
                                        tr("Edit allocation. Old data:\n%1").arg(logger->infoLog(record))
                                        );
        }
        delete(allocations_dialog);
    }
}

void InventoryTableView::findItem()
{
    if(!this->model()) return;
    QSqlRelationalTableModel *model = qobject_cast<QSqlRelationalTableModel *>(this->model());
    QModelIndex index = this->selectionModel()->currentIndex();
    QSqlRecord record = model->record(index.row());
    if(!record.isEmpty()){
        if(model->tableName() == "allocations"){
            QSqlQuery* query = new QSqlQuery();
            if(!query->exec(QString("SELECT `item_id` FROM allocations WHERE `id`='%1'")
                            .arg(record.value("id").toInt())
                            )){
                if(logger) logger->writeLog(Logger::Error, Logger::Allocations,
                                            tr("Sql error:\n%1").arg(query->lastError().text())
                                            );
                return;
            }
            if(!query->first()) return;
            emit searchItem(query->value(0).toInt(), "items");
        }
        else if(model->tableName() == "items" || model->tableName() == "temp_reference_items" || model->tableName() == "temp_reference_items_properties"){
            emit searchItem(record.value("id").toInt(), "items");
        }
    }
}

void InventoryTableView::findAllocation()
{
    if(!this->model()) return;
    QSqlRelationalTableModel *model = qobject_cast<QSqlRelationalTableModel *>(this->model());
    QModelIndex index = this->selectionModel()->currentIndex();
    QSqlRecord record = model->record(index.row());
    if(!record.isEmpty()){
        if(model->tableName() == "items" ||
           model->tableName() == "temp_reference_items" ||
           model->tableName() == "temp_reference_items_properties"){
            int item_id = record.value("id").toInt();

            QSqlQuery* query = new QSqlQuery();
            if(!query->exec(QString("SELECT MAX(`id`) FROM allocations WHERE `item_id`='%1'")
                            .arg(item_id)
                            )){
                if(logger) logger->writeLog(Logger::Error, Logger::Items,
                                            tr("Sql error:\n%1").arg(query->lastError().text())
                                            );
                return;
            }
            if(!query->first()) return;
            emit searchItem(query->value(0).toInt(), "allocations");
        }
    }
}

void InventoryTableView::copyItem()
{
    if(!this->checkUserRights(4)) return;
    if(!this->model()) return;
    QSqlRelationalTableModel *model = qobject_cast<QSqlRelationalTableModel *>(this->model());
    QModelIndex index = this->selectionModel()->currentIndex();
    QSqlRecord record = model->record(index.row());
    if(!record.isEmpty()){
        Items* items_dialog = new Items(this);
        if(logger) items_dialog->setLogger(logger);
        items_dialog->loadItem(record.value("id").toInt());
        items_dialog->setEditMode(false);
        if(items_dialog->exec() == QDialog::Accepted){
            emit tableUpdate("Items");
            if(logger) logger->writeLog(Logger::Add, Logger::Items,
                                        tr("Copy item. From data:\n%1").arg(logger->infoLog(record))
                                        );
        }
        delete items_dialog;
    }
}

void InventoryTableView::scrapItem()
{
    if(!this->checkUserRights(32)) return;
    if(!this->model()) return;
    QSqlRelationalTableModel *model = qobject_cast<QSqlRelationalTableModel *>(this->model());
    QModelIndex index = this->selectionModel()->currentIndex();
    QSqlRecord record = model->record(index.row());
    if(!record.isEmpty()){
        Scrap* scrap_form = new Scrap(this);
        scrap_form->setItem(record);
        if(scrap_form->exec() == QDialog::Accepted){
            QSqlQuery* query = new QSqlQuery(QSqlDatabase::database());

            //            int item_id = query->lastInsertId().toInt();
            if(!query->exec(QString("SELECT * FROM allocations WHERE `item_id` = '%1'")
                    .arg(record.value("id").toString())
                    )){
                if(logger) logger->writeLog(Logger::Error, Logger::Allocations,
                                             tr("Can not load scrapped item info.\n%1").arg(query->lastError().text())
                                            );
                return;
            }
            QString allocations;
            while(query->next()){
                allocations.append(query->value(query->record().indexOf("date")).toString());
                QSqlQuery* q = new QSqlQuery(QSqlDatabase::database());
                if(!q->exec(QString("SELECT `location` FROM locations WHERE `id` = '%1'")
                        .arg(query->value(query->record().indexOf("from_location_id")).toInt())
                        )){
                    if(logger) logger->writeLog(Logger::Error, Logger::Locations,
                                                 tr("Can not load scrapped item info.\n%1").arg(query->lastError().text())
                                                );
                    return;
                }
                q->first();
                allocations.append("|");
                allocations.append(q->value(0).toString());
                if(!q->exec(QString("SELECT `location` FROM locations WHERE `id` = '%1'")
                        .arg(query->value(query->record().indexOf("to_location_id")).toInt())
                        )){
                    if(logger) logger->writeLog(Logger::Error, Logger::Locations,
                                                 tr("Can not load scrapped item info.\n%1").arg(query->lastError().text())
                                                );
                    return;
                }
                q->first();
                allocations.append("=>");
                allocations.append(q->value(0).toString());
                if(!q->exec(QString("SELECT `name` FROM operators WHERE `id` = '%1'")
                        .arg(query->value(query->record().indexOf("from_operator_id")).toInt())
                        )){
                    if(logger) logger->writeLog(Logger::Error, Logger::Operators,
                                                 tr("Can not load scrapped item info.\n%1").arg(query->lastError().text())
                                                );
                    return;
                }
                q->first();
                allocations.append("|");
                allocations.append(q->value(0).toString());
                if(!q->exec(QString("SELECT `name` FROM operators WHERE `id` = '%1'")
                        .arg(query->value(query->record().indexOf("to_operator_id")).toInt())
                        )){
                    if(logger) logger->writeLog(Logger::Error, Logger::Operators,
                                                 tr("Can not load scrapped item info.\n%1").arg(query->lastError().text())
                                                );
                    return;
                }
                q->first();
                allocations.append("=>");
                allocations.append(q->value(0).toString());
                allocations.append("|");
                allocations.append(query->value(query->record().indexOf("note")).toString());
                allocations.append("\n");

                if(!q->exec(QString("DELETE FROM allocations WHERE `id`='%1'")
                            .arg(query->value(query->record().indexOf("id")).toInt())
                            )){
                    if(logger) logger->writeLog(Logger::Error, Logger::Allocations,
                                                 tr("Can not remove scrapped item from allocations.\n%1").arg(query->lastError().text())
                                                );
                    return;
                }
            }

            if(!query->exec(QString("INSERT INTO scrap (`item_id`,`type`,`number`,`location`,`IP`,`operator`,`manufacturer`,`model`,`serialno`,`note`,`allocations_data`,`reason`) "
                                    "VALUES ('%1', '%2', '%3', '%4', '%5', '%6', '%7', '%8', '%9', '%10', '%11', '%12')")
                            .arg(record.value("id").toString())
                            .arg(record.value("type").toString())
                            .arg(record.value("number").toString())
                            .arg(record.value("location").toString())
                            .arg(record.value("IP").toString())
                            .arg(record.value("name").toString())
                            .arg(record.value("manufacturer").toString())
                            .arg(record.value("model").toString())
                            .arg(record.value("serialno").toString())
                            .arg(record.value("note").toString())
                            .arg(allocations)
                            .arg(scrap_form->getReason())
                            )){
                if(logger) logger->writeLog(Logger::Error, Logger::Scrap,
                                             tr("Can not save scrapped item.\n%1").arg(query->lastError().text())
                                            );
                return;
            }
            else{
                if(logger) logger->writeLog(Logger::Add, Logger::Scrap,
                                            tr("Scrapped item:\n%1 %2")
                                            .arg(logger->infoLog(record))
                                            .arg(allocations)
                                            );
            }

            if(!query->exec(QString("DELETE FROM items WHERE `id`='%1'")
                            .arg(record.value("id").toString())
                            )){
                if(logger) logger->writeLog(Logger::Error, Logger::Items,
                                             tr("Can not delete item data from table.\n%1").arg(query->lastError().text())
                                            );
            }
            emit tableUpdate("items");
        }
        delete scrap_form;
    }
}

void InventoryTableView::markRow()
{
    QSqlRelationalTableModel *model = qobject_cast<QSqlRelationalTableModel *>(this->model());
    if(model){
        QModelIndexList list = this->selectionModel()->selectedRows();
        for (int i = 0; i < list.size(); ++i) {
             QModelIndex index = list.at(i);
             if(index.isValid()){
                 this->setItemDelegateForRow(index.row(), new CheckDelegate(this));
                 this->marked << model->record(index.row()).value("id").toString();
             }
         }
        /*
        QModelIndex index = this->selectionModel()->currentIndex();
        if(index.isValid()){
            this->setItemDelegateForRow(index.row(), new CheckDelegate(this));
            this->marked << model->record(index.row()).value("id").toString();
            saveMarked();
            emit tableUpdate(model->tableName());
        }
        */
        saveMarked();
        emit tableUpdate(model->tableName());
    }
}

void InventoryTableView::unmarkRow()
{
    QSqlRelationalTableModel *model = qobject_cast<QSqlRelationalTableModel *>(this->model());
    if(model){
        QModelIndexList list = this->selectionModel()->selectedRows();
        for (int i = 0; i < list.size(); ++i) {
             QModelIndex index = list.at(i);
             if(index.isValid()){
                 this->setItemDelegateForRow(index.row(), new QSqlRelationalDelegate(this));
                 this->marked.removeOne(model->record(index.row()).value("id").toString());
             }
        }
        /*
        QModelIndex index = this->selectionModel()->currentIndex();
        if(index.isValid()){
            this->setItemDelegateForRow(index.row(), new QSqlRelationalDelegate(this));
            this->marked.removeOne(model->record(index.row()).value("id").toString());

            saveMarked();
            emit tableUpdate(model->tableName());
        }
        */
        saveMarked();
        emit tableUpdate(model->tableName());
    }
}

void InventoryTableView::saveMarked()
{
    QSqlRelationalTableModel *model = qobject_cast<QSqlRelationalTableModel *>(this->model());
    if(model){
        marked.removeDuplicates();
        QSqlQuery* query = new QSqlQuery();
        /*if(!query->exec(QString("UPDATE marked SET `list`='%1' WHERE `tablename`='%2'")*/
        if(!query->exec(QString("INSERT OR REPLACE INTO marked (`tablename`, `list`) VALUES( '%1', '%2')")
                        .arg(model->tableName())
                        .arg(this->marked.join(","))
                                )){
             if(logger) logger->writeLog(Logger::Error, Logger::Other,
                                                tr("Sql error:\n%1").arg(query->lastError().text())
                                                );
        }
    }
}

void InventoryTableView::restoreMarked()
{
    QSqlRelationalTableModel *model = qobject_cast<QSqlRelationalTableModel *>(this->model());
    if(!model){
        return;
    }

    while (model->canFetchMore())
         model->fetchMore();

    if(this->marked.size() <= 0) return;
    for(int n=0; n<model->rowCount(); n++){
        QSqlRecord record = model->record(n);
        if(!record.isEmpty()){
            if(this->marked.contains(record.value("id").toString()))
                this->setItemDelegateForRow(n, new CheckDelegate(this));
            else
                this->setItemDelegateForRow(n, new QSqlRelationalDelegate(this));
        }
    }
    if(hide_marked->isChecked()) hideMarked(true);
    if(hide_unmarked->isChecked()) hideUnmarked(true);
}

void InventoryTableView::loadMarked()
{
    QSqlRelationalTableModel *model = qobject_cast<QSqlRelationalTableModel *>(this->model());
    if(model){
        QSqlQuery* query = new QSqlQuery();
        if(!query->exec(QString("SELECT `list` FROM marked WHERE `tablename`='%1'")
                        .arg(model->tableName())
                        )){
            if(logger) logger->writeLog(Logger::Error, Logger::Other,
                                               tr("Sql error:\n%1").arg(query->lastError().text())
                                               );
        }
        if(!query->first()) return;
        this->marked = query->value(0).toString().split(",");

        restoreMarked();
    }
}


void InventoryTableView::cleanMarked()
{
    QSqlRelationalTableModel *model = qobject_cast<QSqlRelationalTableModel *>(this->model());
    if(!model){
        return;
    }

    while (model->canFetchMore())
         model->fetchMore();

    for(int n=0; n<model->rowCount(); n++){
        this->setItemDelegateForRow(n, new QSqlRelationalDelegate(this));
    }

 }

void InventoryTableView::markingReverse()
{
    QSqlRelationalTableModel *model = qobject_cast<QSqlRelationalTableModel *>(this->model());
    if(!model){
        return;
    }

    while (model->canFetchMore())
         model->fetchMore();

    for(int n=0; n<model->rowCount(); n++){
        QSqlRecord record = model->record(n);
        if(!record.isEmpty()){
            if(this->marked.contains(record.value("id").toString())){
                this->setItemDelegateForRow(n, new QSqlRelationalDelegate(this));
                this->marked.removeOne(record.value("id").toString());
            }
            else{
                this->setItemDelegateForRow(n, new CheckDelegate(this));
                this->marked << record.value("id").toString();
            }
        }
    }
    saveMarked();
    emit tableUpdate(model->tableName());
}

void InventoryTableView::hideMarked(bool hide)
{
    QSqlRelationalTableModel *model = qobject_cast<QSqlRelationalTableModel *>(this->model());
    if(!model){
        return;
    }
    while (model->canFetchMore())
         model->fetchMore();

    for(int n=0; n<model->rowCount(); n++){
        QSqlRecord record = model->record(n);
        if(!record.isEmpty()){
            if(this->marked.contains(record.value("id").toString())){
                hide ? this->hideRow(n) : this->showRow(n);
            }
        }
    }
}

void InventoryTableView::hideUnmarked(bool hide)
{
    QSqlRelationalTableModel *model = qobject_cast<QSqlRelationalTableModel *>(this->model());
    if(!model){
        return;
    }

    while (model->canFetchMore())
         model->fetchMore();

    for(int n=0; n<model->rowCount(); n++){
        QSqlRecord record = model->record(n);
        if(!record.isEmpty()){
            if(!this->marked.contains(record.value("id").toString())){
                hide ? this->hideRow(n) : this->showRow(n);
            }
        }
    }

}

void InventoryTableView::printPreviewTable()
{
    #ifndef QT_NO_PRINTER
    if (!model() ) return;
    QSqlRelationalTableModel *model = qobject_cast<QSqlRelationalTableModel *>(this->model());
    QPrinter* printer = new QPrinter();
    this->loadPrinterSettings(printer);
    QPrintPreviewDialog* preview = new QPrintPreviewDialog(printer, this, Qt::WindowMinMaxButtonsHint);

    QMessageBox msgBox;
    msgBox.setText(tr("Would you like to print the entire table?"));
    QPushButton *selectButton = msgBox.addButton(tr("Selection"), QMessageBox::NoRole);
    QPushButton *allButton = msgBox.addButton(tr("Entire table"), QMessageBox::YesRole);

    msgBox.exec();

    if (msgBox.clickedButton() == selectButton) {
         // selection
        connect(preview, SIGNAL(paintRequested(QPrinter*)), this, SLOT(printSelection(QPrinter*)));
    }else if (msgBox.clickedButton() == allButton) {
         // entire table
        connect(preview, SIGNAL(paintRequested(QPrinter*)), this, SLOT(printTable(QPrinter*)));
    }


    //connect(preview, SIGNAL(paintRequested(QPrinter*)), this, SLOT(printTable(QPrinter*)));

    if(model)
        preview->setWindowTitle(tr("Print preview: %1")
                            .arg(model->tableName())
                            );
    else
        preview->setWindowTitle(tr("Print preview"));
    preview->exec();
    this->savePrinterSettings(printer);

    #endif
}

void InventoryTableView::printTable(QPrinter *printer)
{
        #ifndef QT_NO_PRINTER
        //QPrintDialog printDialog(printer, this);
        //printDialog.exec();
        QHeaderView* hHeader = horizontalHeader();
        QHeaderView* vHeader = verticalHeader();

        QPainter painter(printer);
        // Check that there is a valid device to print to.
        if (!painter.isActive()) return;
        int pageLeft = 0;
        int pageTop = 0;
        int pageHeight = painter.device()->height();
        int pageWidth = painter.device()->width();
        int x = pageLeft;
        int y = pageTop;
        QList<QVector<QLine> > linePages;
        QVector<QLine> verticalLines;
        QList<QList<int> > sectionPages;
        QList<int> sections;
        QList<QList<int> > widthPages;
        QList<int> widths;
        QList<int> pageRights;
        QList<QStringList> pageLabels;
        QStringList labels;
        int logicalIndex;

        // Перечисляем столбцы
        for (int i = 0; i < hHeader->count(); ++i) {
            logicalIndex = hHeader->logicalIndex(i);
            // Если столбец не скрыт
            if (!hHeader->isSectionHidden(logicalIndex)) {
            // Если уже не помещаемся на страницу пошем новую страницу
                if (pageWidth <= x + hHeader->sectionSize(logicalIndex)) {
                    verticalLines.append(QLine(x, pageTop, x, pageHeight));
                    pageRights.append(x);
                    x = pageLeft;
                    sectionPages.append(sections);
                    sections.clear();
                    linePages.append(verticalLines);
                    verticalLines.clear();
                    widthPages.append(widths);
                    widths.clear();
                    pageLabels.append(labels);
                    labels.clear();
                }
                // Добавляем, так как колонка видимая
                sections.append(logicalIndex);
                widths.append(hHeader->sectionSize(logicalIndex));
                labels.append(model()->headerData(logicalIndex, Qt::Horizontal).toString());
                verticalLines.append(QLine(x, pageTop, x, pageHeight));
                // Смещаемся правее на ширину колонки
                x += hHeader->sectionSize(logicalIndex);
            }
        }

        if (!sections.isEmpty()) {
            verticalLines.append(QLine(x, pageTop, x, pageHeight));
            pageRights.append(x);
            x = pageLeft;
            sectionPages.append(sections);
            sections.clear();
            linePages.append(verticalLines);
            verticalLines.clear();
            widthPages.append(widths);
            widths.clear();
            pageLabels.append(labels);
            labels.clear();
        }

        int columnPageCount = sectionPages.count();
        int columnCount = sections.count();
        int rowCount = model()->rowCount(rootIndex());
        int rowHeight;
        int columnWidth;
        QModelIndex currentIndex;
        int row = 0;
        int lastPageRow = 0;
        int columnPageIndex = 0;
        int rightColumn;

        while (true) {
            // Получаем колонки текущей страницы
            sections = sectionPages.at(columnPageIndex);
            widths = widthPages.at(columnPageIndex);
            verticalLines = linePages.at(columnPageIndex);
            // Количество колонок текущей страницы
            columnCount = sections.count();
            rightColumn = pageRights.at(columnPageIndex);
            labels = pageLabels.at(columnPageIndex);
            // Перемещаемся в левый верхний угол страницы
            x = pageLeft;
            y = pageTop;
            rowHeight = hHeader->height();

            // Рисуем колонки
            for (int i = 0; i < columnCount; ++i) {
                // Текущее значение
                columnWidth = widths.at(i);
                painter.drawText(x, y, columnWidth, rowHeight
                                 , currentIndex.data(Qt::TextAlignmentRole).toInt()
                                   | Qt::TextWordWrap
                                   | Qt::AlignVCenter
                                   | Qt::AlignHCenter
                                   , labels.at(i)
                                  );
                painter.drawLine(pageLeft, y, rightColumn, y);
                // Перемащаемся правее
                x += widths.at(i);
             }

             y += rowHeight;
             x = pageLeft;

             // Пока можем рисовать на текущей странице
             while (true) {
                // Высота текущей строки
                rowHeight = vHeader->sectionSize(row);

                if (y + rowHeight > pageHeight) {
                    //--row;
                    break;
                }

                // Рисуем колонки
                for (int i = 0; i < columnCount; ++i) {
                    // Текущее значение
                    currentIndex = model()->index(row, sections.at(i), rootIndex());
                    columnWidth = widths.at(i);

                    if (!currentIndex.data().isNull())
                        painter.drawText(x, y, columnWidth, rowHeight
                                       , currentIndex.data(Qt::TextAlignmentRole).toInt()
                                       | Qt::TextWordWrap
                                       | Qt::AlignVCenter
                                       | Qt::AlignHCenter
                                       , currentIndex.data().toString()
                                      );

                     painter.drawLine(pageLeft, y, rightColumn, y);
                     // Перемащаемся правее
                     x += widths.at(i);
                  }

                  x = pageLeft;
                  // Перемещаемся ниже
                  y += rowHeight;
                  // Берем следующую строку
                  ++row;

                  // Если строки кончились
                  if (row >= rowCount)
                    break;
                }

                // Рисуем завершающую горизонтальную линию
                painter.drawLine(pageLeft, y, rightColumn, y);
                // Рисуем вертикальные линии
                painter.drawLines(verticalLines);
                // Вытираем лишние концы
                // TODO: hard-coded background color for page
                painter.fillRect(pageLeft, y + 1, pageWidth, pageHeight - (y + 1), QBrush(Qt::white));

                // Если предыдущие колонки не поместились
                if (columnPageIndex < columnPageCount - 1) {
                    ++columnPageIndex;
                    row = lastPageRow;
                // Предыдущие колонки поместились
                } else {
                    columnPageIndex = 0;
                    lastPageRow = row;

                    // Строки закончились
                    if (row >= rowCount)
                        break;
                }

                // Создаем новую страницу
                printer->newPage();
        }
        #endif
}

void InventoryTableView::printSelection(QPrinter *printer)
{
    #ifndef QT_NO_PRINTER

    QModelIndexList list = selectionModel()->selectedIndexes();
    if(list.isEmpty()) return;


    QHeaderView* hHeader = horizontalHeader();
    QHeaderView* vHeader = verticalHeader();

    QPainter painter(printer);
        // Check that there is a valid device to print to.
    if (!painter.isActive()) return;

    int pageLeft = 0;
    int pageTop = 0;
    int pageHeight = painter.device()->height();
    int pageWidth = painter.device()->width();
    int x = pageLeft;
    int y = pageTop;
    QList<QVector<QLine> > linePages;
    QVector<QLine> verticalLines;
    QList<QList<int> > sectionPages;
    QList<int> sections;
    QList<QList<int> > widthPages;
    QList<int> widths;
    QList<int> pageRights;
    QList<QStringList> pageLabels;
    QStringList labels;
    int logicalIndex;

    // Перечисляем столбцы
    for (int i = 0; i < hHeader->count(); ++i) {
        logicalIndex = hHeader->logicalIndex(i);
        // Если столбец не скрыт
        if (!hHeader->isSectionHidden(logicalIndex)) {
        // Если уже не помещаемся на страницу пошем новую страницу
            if (pageWidth <= x + hHeader->sectionSize(logicalIndex)) {
                verticalLines.append(QLine(x, pageTop, x, pageHeight));
                pageRights.append(x);
                x = pageLeft;
                sectionPages.append(sections);
                sections.clear();
                linePages.append(verticalLines);
                verticalLines.clear();
                widthPages.append(widths);
                widths.clear();
                pageLabels.append(labels);
                labels.clear();
            }
            // Добавляем, так как колонка видимая
            sections.append(logicalIndex);
            widths.append(hHeader->sectionSize(logicalIndex));
            labels.append(model()->headerData(logicalIndex, Qt::Horizontal).toString());
            verticalLines.append(QLine(x, pageTop, x, pageHeight));
            // Смещаемся правее на ширину колонки
            x += hHeader->sectionSize(logicalIndex);
        }
    }

    if (!sections.isEmpty()) {
        verticalLines.append(QLine(x, pageTop, x, pageHeight));
        pageRights.append(x);
        x = pageLeft;
        sectionPages.append(sections);
        sections.clear();
        linePages.append(verticalLines);
        verticalLines.clear();
        widthPages.append(widths);
        widths.clear();
        pageLabels.append(labels);
        labels.clear();
    }

    int columnPageCount = sectionPages.count();
    int columnCount = sections.count();
    // int rowCount = model()->rowCount(rootIndex());
    int rowCount = selectionModel()->selectedRows().count();

    int rowHeight;
    int columnWidth;
    QModelIndex currentIndex;
    int row = 0;
    int lastPageRow = 0;
    int columnPageIndex = 0;
    int rightColumn;

        while (true) {
            // Получаем колонки текущей страницы
            sections = sectionPages.at(columnPageIndex);
            widths = widthPages.at(columnPageIndex);
            verticalLines = linePages.at(columnPageIndex);
            // Количество колонок текущей страницы
            columnCount = sections.count();
            rightColumn = pageRights.at(columnPageIndex);
            labels = pageLabels.at(columnPageIndex);
            // Перемещаемся в левый верхний угол страницы
            x = pageLeft;
            y = pageTop;
            rowHeight = hHeader->height();

            // Рисуем колонки
            for (int i = 0; i < columnCount; ++i) {
                // Текущее значение
                columnWidth = widths.at(i);
                painter.drawText(x, y, columnWidth, rowHeight
                                 , currentIndex.data(Qt::TextAlignmentRole).toInt()
                                   | Qt::TextWordWrap
                                   | Qt::AlignVCenter
                                   | Qt::AlignHCenter
                                   , labels.at(i)
                                  );
                painter.drawLine(pageLeft, y, rightColumn, y);
                // Перемащаемся правее
                x += widths.at(i);
             }

             y += rowHeight;
             x = pageLeft;

             // Пока можем рисовать на текущей странице
             while (true) {
                // Высота текущей строки
                rowHeight = vHeader->sectionSize(row);

                if (y + rowHeight > pageHeight) {
                    //--row;
                    break;
                }

                // Рисуем колонки
                for (int i = 0; i < columnCount; i++) {
                    // Текущее значение
                    //currentIndex = model()->index(row, sections.at(i), rootIndex());

                    currentIndex = model()->index(list.at(row).row(), sections.at(i), rootIndex());

                    columnWidth = widths.at(i);

                    if (!currentIndex.data().isNull())
                        painter.drawText(x, y, columnWidth, rowHeight
                                       , currentIndex.data(Qt::TextAlignmentRole).toInt()
                                       | Qt::TextWordWrap
                                       | Qt::AlignVCenter
                                       | Qt::AlignHCenter
                                       , currentIndex.data().toString()
                                      );

                     painter.drawLine(pageLeft, y, rightColumn, y);
                     // Перемащаемся правее
                     x += widths.at(i);
                  }

                  x = pageLeft;
                  // Перемещаемся ниже
                  y += rowHeight;
                  // Берем следующую строку
                  ++row;

                  // Если строки кончились
                  if (row >= rowCount)
                    break;
                }

                // Рисуем завершающую горизонтальную линию
                painter.drawLine(pageLeft, y, rightColumn, y);
                // Рисуем вертикальные линии
                painter.drawLines(verticalLines);
                // Вытираем лишние концы
                // TODO: hard-coded background color for page
                painter.fillRect(pageLeft, y + 1, pageWidth, pageHeight - (y + 1), QBrush(Qt::white));

                // Если предыдущие колонки не поместились
                if (columnPageIndex < columnPageCount - 1) {
                    ++columnPageIndex;
                    row = lastPageRow;
                // Предыдущие колонки поместились
                } else {
                    columnPageIndex = 0;
                    lastPageRow = row;

                    // Строки закончились
                    if (row >= rowCount)
                        break;
                }

                // Создаем новую страницу
                printer->newPage();
        }
        #endif
}

void InventoryTableView::savePrinterSettings(QPrinter *printer)
{
    QSqlRelationalTableModel *model = qobject_cast<QSqlRelationalTableModel *>(this->model());
    if(!model) return;
#ifndef QT_NO_PRINTER
    qreal left;
    qreal top;
    qreal right;
    qreal bottom;
    QPrinter::Unit unit = QPrinter::Millimeter;

    printer->getPageMargins(&left, &top, &right, &bottom, unit);

    QSettings settings(qApp->property("ini").toString(), QSettings::IniFormat);
    settings.beginGroup(QString("PrinterSettings_%1").arg(model->tableName()));
    settings.setValue("Orientation", printer->orientation());
    settings.setValue("ColorMode", printer->colorMode());
    settings.setValue("PaperSize", printer->paperSize());
    settings.setValue("left", left);
    settings.setValue("top", top);
    settings.setValue("right", right);
    settings.setValue("bottom", bottom);
    settings.setValue("Unit", unit);
    settings.endGroup();

#endif
}

void InventoryTableView::loadPrinterSettings(QPrinter *printer)
{
    QSqlRelationalTableModel *model = qobject_cast<QSqlRelationalTableModel *>(this->model());
    if(!model) return;
#ifndef QT_NO_PRINTER

    //QDir appdir = qApp->applicationDirPath();
    //QSettings settings(appdir.filePath("Inventory.ini"), QSettings::IniFormat);
    QSettings settings(qApp->property("ini").toString(), QSettings::IniFormat);
    settings.beginGroup(QString("PrinterSettings_%1").arg(model->tableName()));

    switch(settings.value("Orientation").toInt()){
    case 1: printer->setOrientation(QPrinter::Landscape); break;
    default: printer->setOrientation(QPrinter::Portrait); break;
    }
    switch(settings.value("ColorMode").toInt()){
    case 0: printer->setColorMode(QPrinter::Color);
    default: printer->setColorMode(QPrinter::GrayScale);
    }
    printer->setPageMargins(
                settings.value("left", 15.00).toDouble(),
                settings.value("top", 15.00).toDouble(),
                settings.value("right").toDouble(),
                settings.value("bottom").toDouble(),
                QPrinter::Millimeter
                );
    settings.endGroup();

#endif
}

void InventoryTableView::addProperty()
{
    if(!this->checkUserRights(19)) return;
    emit add_property_();
}

void InventoryTableView::editProperty()
{
    if(!this->checkUserRights(19)) return;
    if(!this->model()) return;
    QSqlRelationalTableModel *model = qobject_cast<QSqlRelationalTableModel *>(this->model());
    QModelIndex index = this->selectionModel()->currentIndex();
    QSqlRecord record = model->record(index.row());
    if(!record.isEmpty()){
        bool ok;
        QString text = QInputDialog::getText(this, tr("Edit property"),
                                             tr("Property name:"), QLineEdit::Normal,
                                             record.value("property").toString(), &ok);
        if (ok && !text.isEmpty()){
            QSqlQuery* query = new QSqlQuery();
            if(!query->exec(QString("UPDATE properties SET `property`='%1' WHERE `id`='%2'")
                            .arg(text)
                            .arg(record.value("id").toInt())
                            )){
                if(logger) logger->writeLog(Logger::Error, Logger::Properties,
                                            tr("Can not update database. Sql error:\n%1").arg(query->lastError().text())
                                            );
            }
            else emit tableUpdate("properties");
        }
    }
}

void InventoryTableView::deleteProperty()
{
    if(!this->checkUserRights(20)) return;
    if(!this->model()) return;
    QSqlRelationalTableModel *model = qobject_cast<QSqlRelationalTableModel *>(this->model());
    QModelIndex index = this->selectionModel()->currentIndex();
    QSqlRecord record = model->record(index.row());
    if(!record.isEmpty()){
        QString property = record.value("property").toString();
        int ret = QMessageBox::question(this, tr("Properties"),
                tr("Do you want to delete %1?\nThis could not be restored!")
                                    .arg(property),
                                    QMessageBox::Ok | QMessageBox::Cancel);
        if(ret == QMessageBox::Cancel) return;
        QString info;
        if(logger) info = logger->infoLog(record);
        QSqlQuery* query = new QSqlQuery(QSqlDatabase::database());
        if(!query->exec(QString("DELETE FROM properties WHERE `id`=%1")
                            .arg(record.value("id").toInt()))
            ){
            if(logger) logger->writeLog(Logger::Error, Logger::Attachments,
                                        tr("Can not remove from database. Sql error:\n%1").arg(query->lastError().text())
                                        );
        }
        else{
            if(logger) logger->writeLog(Logger::Delete, Logger::Properties,
                                        tr("Delete property info:\n%1").arg(info));
        }
        if(!query->exec(QString("DELETE FROM item_properties WHERE `property_id`='%1'")
                        .arg(record.value("id").toInt()))){
            if(logger) logger->writeLog(Logger::Error, Logger::Properties,
                                        tr("Sql error:\n%1").arg(query->lastError().text())
                                        );
        }
        emit tableUpdate("properties");
    }
}

void InventoryTableView::dropEvent(QDropEvent *event)
{
    MySqlRelationalTableModel *model = qobject_cast<MySqlRelationalTableModel *>(this->model());
    if(model){
        if(model->tableName() == "attachments" && event->mimeData()->hasUrls() == true){
            emit(add_droped_files(event->mimeData()));
        }
    }
    event->acceptProposedAction();
}


#include "rights.h"
#include "ui_rights.h"
#include "logger.h"

#include <QtSql>

Rights::Rights(QWidget *parent,  Qt::WindowFlags f) : QDialog(parent, f), ui(new Ui::Rights)
{
    ui->setupUi(this);
    setId(0);
    logger = 0;
}

Rights::~Rights()
{
    delete ui;
}

void Rights::loadRights()
{
    if(!_id) return;
    QSqlQuery* query = new QSqlQuery(QSqlDatabase::database());
    if(!query->exec(QString("SELECT * FROM `users_rights` WHERE `id`=%1")
                        .arg(getId()))){
        if(logger) logger->writeLog(Logger::Error, Logger::Rights,
                                    tr("Can not get rights info from database. Sql error:\n%1")
                                    .arg(query->lastError().text())
                                    );
        return;
    }
    if(!query->first()){
        if(logger) logger->writeLog(Logger::Error, Logger::Rights, tr("Empty rights table"));
        return;
    }
    QSqlRecord record = query->record();

    ui->attachments_view_checkBox->setChecked(record.value("attachments_view").toBool());
    ui->attachments_edit_checkBox->setChecked(record.value("attachments_edit").toBool());
    ui->attachments_delete_checkBox->setChecked(record.value("attachments_delete").toBool());

    ui->items_view_checkBox->setChecked(record.value("items_view").toBool());
    ui->items_edit_checkBox->setChecked(record.value("items_edit").toBool());
    ui->items_delete_checkBox->setChecked(record.value("items_delete").toBool());

    ui->item_types_view_checkBox->setChecked(record.value("item_types_view").toBool());
    ui->item_types_edit_checkBox->setChecked(record.value("item_types_edit").toBool());
    ui->item_types_delete_checkBox->setChecked(record.value("item_types_delete").toBool());

    ui->item_status_view_checkBox->setChecked(record.value("item_status_view").toBool());
    ui->item_status_edit_checkBox->setChecked(record.value("item_status_edit").toBool());
    ui->item_status_delete_checkBox->setChecked(record.value("item_status_delete").toBool());

    ui->users_view_checkBox->setChecked(record.value("users_view").toBool());
    ui->users_edit_checkBox->setChecked(record.value("users_edit").toBool());
    ui->users_delete_checkBox->setChecked(record.value("users_delete").toBool());

    ui->operators_view_checkBox->setChecked(record.value("operators_view").toBool());
    ui->operators_edit_checkBox->setChecked(record.value("operators_edit").toBool());
    ui->operators_delete_checkBox->setChecked(record.value("operators_delete").toBool());

    ui->locations_view_checkBox->setChecked(record.value("locations_view").toBool());
    ui->locations_edit_checkBox->setChecked(record.value("locations_edit").toBool());
    ui->locations_delete_checkBox->setChecked(record.value("locations_delete").toBool());

    ui->allocations_view_checkBox->setChecked(record.value("allocations_view").toBool());
    ui->allocations_edit_checkBox->setChecked(record.value("allocations_edit").toBool());
    ui->allocations_add_checkBox->setChecked(record.value("allocations_add").toBool());

    ui->departments_view_checkBox->setChecked(record.value("departments_view").toBool());
    ui->departments_edit_checkBox->setChecked(record.value("departments_edit").toBool());
    ui->departments_delete_checkBox->setChecked(record.value("departments_delete").toBool());

    ui->logs_view_checkBox->setChecked(record.value("logs_view").toBool());
    ui->logs_delete_checkBox->setChecked(record.value("logs_delete").toBool());

    ui->sql_view_checkBox->setChecked(record.value("sql_view").toBool());
    ui->sql_edit_checkBox->setChecked(record.value("sql_edit").toBool());

    ui->scrap_view_checkBox->setChecked(record.value("scrap_view").toBool());
    ui->scrap_edit_checkBox->setChecked(record.value("scrap_edit").toBool());
    ui->scrap_delete_checkBox->setChecked(record.value("scrap_delete").toBool());
}

void Rights::accept()
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    QSqlQuery* query = new QSqlQuery(QSqlDatabase::database());
    if(!query->exec(QString("UPDATE `users_rights` SET "
                            "`attachments_view`=%1, "
                            "`attachments_edit`=%2, "
                            "`attachments_delete`=%3 "
                            "WHERE `id`=%4")
                    .arg(ui->attachments_view_checkBox->isChecked())
                    .arg(ui->attachments_edit_checkBox->isChecked())
                    .arg(ui->attachments_delete_checkBox->isChecked())
                    .arg(getId()))){
        if(logger) logger->writeLog(Logger::Error, Logger::Rights,
                                    tr("Sql error:\n%1")
                                    .arg(query->lastError().text())
                                    );
    }

    if(!query->exec(QString("UPDATE `users_rights` SET "
                            "`users_view`=%1, "
                            "`users_edit`=%2, "
                            "`users_delete`=%3 "
                            "WHERE `id`=%4")
                    .arg(ui->users_view_checkBox->isChecked())
                    .arg(ui->users_edit_checkBox->isChecked())
                    .arg(ui->users_delete_checkBox->isChecked())
                    .arg(getId()))){
        if(logger) logger->writeLog(Logger::Error, Logger::Rights,
                                    tr("Sql error:\n%1")
                                    .arg(query->lastError().text())
                                    );
    }

    if(!query->exec(QString("UPDATE `users_rights` SET "
                            "`operators_view`=%1, "
                            "`operators_edit`=%2, "
                            "`operators_delete`=%3 "
                            "WHERE `id`=%4")
                    .arg(ui->operators_view_checkBox->isChecked())
                    .arg(ui->operators_edit_checkBox->isChecked())
                    .arg(ui->operators_delete_checkBox->isChecked())
                    .arg(getId()))){
        if(logger) logger->writeLog(Logger::Error, Logger::Rights,
                                    tr("Sql error:\n%1")
                                    .arg(query->lastError().text())
                                    );
    }

    if(!query->exec(QString("UPDATE `users_rights` SET "
                            "`items_view`=%1, "
                            "`items_edit`=%2, "
                            "`items_delete`=%3 "
                            "WHERE `id`=%4")
                    .arg(ui->items_view_checkBox->isChecked())
                    .arg(ui->items_edit_checkBox->isChecked())
                    .arg(ui->items_delete_checkBox->isChecked())
                    .arg(getId()))){
        if(logger) logger->writeLog(Logger::Error, Logger::Rights,
                                    tr("Sql error:\n%1")
                                    .arg(query->lastError().text())
                                    );
    }

    if(!query->exec(QString("UPDATE `users_rights` SET "
                            "`item_types_view`=%1, "
                            "`item_types_edit`=%2, "
                            "`item_types_delete`=%3 "
                            "WHERE `id`=%4")
                    .arg(ui->item_types_view_checkBox->isChecked())
                    .arg(ui->item_types_edit_checkBox->isChecked())
                    .arg(ui->item_types_delete_checkBox->isChecked())
                    .arg(getId()))){
        if(logger) logger->writeLog(Logger::Error, Logger::Rights,
                                    tr("Sql error:\n%1")
                                    .arg(query->lastError().text())
                                    );
    }

    if(!query->exec(QString("UPDATE `users_rights` SET "
                            "`item_status_view`=%1, "
                            "`item_status_edit`=%2, "
                            "`item_status_delete`=%3 "
                            "WHERE `id`=%4")
                    .arg(ui->item_status_view_checkBox->isChecked())
                    .arg(ui->item_status_edit_checkBox->isChecked())
                    .arg(ui->item_status_delete_checkBox->isChecked())
                    .arg(getId()))){
        if(logger) logger->writeLog(Logger::Error, Logger::Rights,
                                    tr("Sql error:\n%1")
                                    .arg(query->lastError().text())
                                    );
    }

    if(!query->exec(QString("UPDATE `users_rights` SET "
                            "`locations_view`=%1, "
                            "`locations_edit`=%2, "
                            "`locations_delete`=%3 "
                            "WHERE `id`=%4")
                    .arg(ui->locations_view_checkBox->isChecked())
                    .arg(ui->locations_edit_checkBox->isChecked())
                    .arg(ui->locations_delete_checkBox->isChecked())
                    .arg(getId()))){
        if(logger) logger->writeLog(Logger::Error, Logger::Rights,
                                    tr("Sql error:\n%1")
                                    .arg(query->lastError().text())
                                    );
    }

    if(!query->exec(QString("UPDATE `users_rights` SET "
                            "`allocations_view`=%1, "
                            "`allocations_edit`=%2, "
                            "`allocations_add`=%3 "
                            "WHERE `id`=%4")
                    .arg(ui->allocations_view_checkBox->isChecked())
                    .arg(ui->allocations_edit_checkBox->isChecked())
                    .arg(ui->allocations_add_checkBox->isChecked())
                    .arg(getId()))){
        if(logger) logger->writeLog(Logger::Error, Logger::Rights,
                                    tr("Sql error:\n%1")
                                    .arg(query->lastError().text())
                                    );
    }

    if(!query->exec(QString("UPDATE `users_rights` SET "
                            "`departments_view`=%1, "
                            "`departments_edit`=%2, "
                            "`departments_delete`=%3 "
                            "WHERE `id`=%4")
                    .arg(ui->departments_view_checkBox->isChecked())
                    .arg(ui->departments_edit_checkBox->isChecked())
                    .arg(ui->departments_delete_checkBox->isChecked())
                    .arg(getId()))){
        if(logger) logger->writeLog(Logger::Error, Logger::Rights,
                                    tr("Sql error:\n%1")
                                    .arg(query->lastError().text())
                                    );
    }

    if(!query->exec(QString("UPDATE `users_rights` SET "
                            "`logs_view`=%1, "
                            "`logs_delete`=%2 "
                            "WHERE `id`=%3")
                    .arg(ui->logs_view_checkBox->isChecked())
                    .arg(ui->logs_delete_checkBox->isChecked())
                    .arg(getId()))){
        if(logger) logger->writeLog(Logger::Error, Logger::Rights,
                                    tr("Sql error:\n%1")
                                    .arg(query->lastError().text())
                                    );
    }

    if(!query->exec(QString("UPDATE `users_rights` SET "
                            "`sql_view`=%1, "
                            "`sql_edit`=%2 "
                            "WHERE `id`=%3")
                    .arg(ui->sql_view_checkBox->isChecked())
                    .arg(ui->sql_edit_checkBox->isChecked())
                    .arg(getId()))){
        if(logger) logger->writeLog(Logger::Error, Logger::Rights,
                                    tr("Sql error:\n%1")
                                    .arg(query->lastError().text())
                                    );
    }

    if(!query->exec(QString("UPDATE `users_rights` SET "
                            "`scrap_view`=%1, "
                            "`scrap_edit`=%2, "
                            "`scrap_delete`=%3 "
                            "WHERE `id`=%4")
                    .arg(ui->scrap_view_checkBox->isChecked())
                    .arg(ui->scrap_edit_checkBox->isChecked())
                    .arg(ui->scrap_delete_checkBox->isChecked())
                    .arg(getId()))){
        if(logger) logger->writeLog(Logger::Error, Logger::Rights,
                                    tr("Sql error:\n%1")
                                    .arg(query->lastError().text())
                                    );
    }

    QApplication::restoreOverrideCursor();
    QDialog::accept();
}

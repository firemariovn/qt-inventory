#include "scrap.h"
#include "ui_scrap.h"

#include <QtSql>

Scrap::Scrap(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Scrap)
{
    ui->setupUi(this);
}

Scrap::~Scrap()
{
    delete ui;
}

void Scrap::setItem(const QSqlRecord &record)
{
    if(!record.isEmpty()){
        ui->number_lineEdit->setText(record.value("number").toString());
        ui->operator_lineEdit->setText(record.value("name").toString());
        ui->type_lineEdit->setText(record.value("type").toString());
        ui->manufacturer_lineEdit->setText(record.value("manufacturer").toString());
    }
}

QString Scrap::getReason() const
{
    return ui->reason_textEdit->toPlainText();
}

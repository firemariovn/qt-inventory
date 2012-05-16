#include "referencesform.h"
#include "ui_referencesform.h"

#include <QtSql>
#include <QtGui>

ReferencesForm::ReferencesForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ReferencesForm)
{
    ui->setupUi(this);

    connect(ui->type_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(fillProperties(int)));

    fillFilters();
}

ReferencesForm::~ReferencesForm()
{
    delete ui;
}

void ReferencesForm::fillFilters()
{
    QSqlTableModel* types_model = new QSqlTableModel(ui->type_comboBox);
    types_model->setTable("item_types");
    types_model->select();

    ui->type_comboBox->setModel(types_model);
    ui->type_comboBox->setModelColumn(types_model->fieldIndex("type"));
    ui->type_comboBox->setCurrentIndex(0);
}

void ReferencesForm::fillProperties(const int type_id)
{
    /*
    QSqlQuery* query = new QSqlQuery();
    if(!query->exec(QString("SELECT `property` FROM properties WHERE `type_id`='%1'")
                    .arg(ui->type_comboBox->itemModelId())
                    )){
        qDebug() << "Sql Error: " << query->lastError().text();
        return;
    }
    ui->properties_comboBox->clear();
    ui->properties_comboBox->addItem(tr("None"));
    while(query->next()){
        ui->properties_comboBox->addItem(query->value(0).toString());
    }
    */

    QSqlTableModel* properties_model = new QSqlTableModel(ui->properties_comboBox);
    properties_model->setTable("properties");
    properties_model->setFilter(QString("type_id=%1").arg(ui->type_comboBox->itemModelId()));
    properties_model->select();

    ui->properties_comboBox->setModel(properties_model);
    ui->properties_comboBox->setModelColumn(properties_model->fieldIndex("property"));
    ui->properties_comboBox->setCurrentIndex(0);
}

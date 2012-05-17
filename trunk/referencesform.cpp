#include "referencesform.h"
#include "ui_referencesform.h"
#include "headerview.h"

#include <QtSql>
#include <QtGui>

ReferencesForm::ReferencesForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ReferencesForm)
{
    ui->setupUi(this);

    model = 0;

    headerView = new HeaderView(Qt::Horizontal);
    ui->tableView->setHorizontalHeader(headerView);

    fillTypes();
    fillProperties(0);
    setFilter();

    connect(ui->type_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(fillProperties(int)));
    connect(ui->properties_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(fillFilter(int)));
    connect(ui->search_pushButton, SIGNAL(clicked()), this, SLOT(setFilter()));
    connect(ui->filter_comboBox, SIGNAL(editTextChanged(QString)), this, SLOT(setFilter()));
}

ReferencesForm::~ReferencesForm()
{
    if(model){
        if(headerView) headerView->saveStateToSettings(model->tableName());
        delete model;
    }
    if(headerView) delete headerView;
}

void ReferencesForm::fillTypes()
{
    QSqlQuery* query = new QSqlQuery();
    if(!query->exec(QString("SELECT `id`,`type` FROM item_types"))){
        qDebug() << "Sql Error: " << query->lastError().text();
        return;
    }
    ui->type_comboBox->clear();
    ui->type_comboBox->addItem(tr("None"), 0);

    while(query->next()){
        ui->type_comboBox->addItem(query->value(1).toString(), query->value(0).toInt());
    }
}

void ReferencesForm::fillProperties(const int index)
{
    QSqlQuery* query = new QSqlQuery();
    if(!query->exec(QString("SELECT `id`,`property` FROM properties WHERE `type_id`='%1'")
                    .arg(ui->type_comboBox->itemData(index).toInt())
                    )){
        qDebug() << "Sql Error: " << query->lastError().text();
        return;
    }
    ui->properties_comboBox->clear();
    ui->properties_comboBox->addItem(tr("None"), 0);
    while(query->next()){
        ui->properties_comboBox->addItem(query->value(1).toString(), query->value(0).toInt());
    }
    setFilter();
}

void ReferencesForm::fillFilter(const int index)
{
    QSqlQuery* query = new QSqlQuery();
    if(!query->exec(QString("SELECT `value` FROM item_properties WHERE `property_id`='%1'")
                    .arg(ui->properties_comboBox->itemData(index).toInt())
                    )){
        qDebug() << "Sql Error: " << query->lastError().text();
        return;
    }
    ui->filter_comboBox->clear();

    QStringList properties;
    while(query->next()){
        properties << query->value(0).toString();
    }
    properties.removeDuplicates();
    ui->filter_comboBox->addItems(properties);

    QCompleter *completer = new QCompleter(properties, this);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    completer->setCaseSensitivity(Qt::CaseInsensitive);

    ui->filter_comboBox->setCompleter(completer);
}

void ReferencesForm::setFilter()
{
    if(model) delete(model);
    model = new QSqlRelationalTableModel(this);
    QSqlQuery* query = new QSqlQuery();
    //QSqlRelationalTableModel* model = new QSqlRelationalTableModel(ui->tableView);

    if(ui->properties_comboBox->itemData(ui->properties_comboBox->currentIndex()).toInt()){
        QString q("CREATE TEMP TABLE IF NOT EXISTS `temp_reference_properties` "
                  "(`id` INTEGER PRIMARY KEY  AUTOINCREMENT  NOT NULL , `property_id` "
                  "INTEGER NOT NULL , `item_id` INTEGER, `value` VARCHAR)");
        if(!query->exec(q)){
            qDebug() << "Sql Error: " << query->lastError().text();
            //return;
        }
        q = "DELETE FROM `temp_reference_properties`";
        if(!query->exec(q)){
            qDebug() << "Sql Error: " << query->lastError().text();
            //return;
        }
        q = QString("CREATE TEMP TABLE IF NOT EXISTS `temp_reference_items_properties` "
            "(`status_id` NUMERIC,`IP` TEXT, `note` TEXT, `location_id` NUMERIC,`model` TEXT, `manufacturer` TEXT, "
            "`serialno` TEXT, `id` INTEGER PRIMARY KEY , `type_id` NUMERIC, `number` TEXT, `operator_id` NUMERIC, "
            "`value` TEXT DEFAULT '')");
        if(!query->exec(q)){
            qDebug() << "Sql Error: " << query->lastError().text();
             //return;
        }
        q = "DELETE FROM `temp_reference_items_properties`";
        if(!query->exec(q)){
            qDebug() << "Sql Error: " << query->lastError().text();
             //return;
        }
        q = QString("INSERT INTO `temp_reference_properties` "
            "SELECT * FROM item_properties WHERE property_id = '%1'")
            .arg(ui->properties_comboBox->itemData(ui->properties_comboBox->currentIndex()).toInt());
        if(!ui->filter_comboBox->currentText().isEmpty()){
            q.append(QString(" AND value LIKE '%%1%'")
                    .arg(ui->filter_comboBox->currentText())
                     );
        }
        if(!query->exec(q)){
            qDebug() << "Sql Error: " << query->lastError().text();
            //return;
        }
        q = QString("INSERT INTO temp_reference_items_properties "
                    "SELECT items.*, temp_reference_properties.value FROM items "
                    "JOIN temp_reference_properties "
                    "ON items.id = temp_reference_properties.item_id");

        if(!query->exec(q)){
            qDebug() << "Sql Error: " << query->lastError().text();
            //return;
        }
        model->setTable("temp_reference_items_properties");
    }

    else {
        QString q = QString("CREATE TEMP TABLE IF NOT EXISTS `temp_reference_items` "
            "(`status_id` NUMERIC,`IP` TEXT, `note` TEXT, `location_id` NUMERIC,`model` TEXT, `manufacturer` TEXT, "
            "`serialno` TEXT, `id` INTEGER PRIMARY KEY , `type_id` NUMERIC, `number` TEXT, `operator_id` NUMERIC)");
        if(!query->exec(q)){
            qDebug() << "Sql Error: " << query->lastError().text();
             //return;
        }
        q = "DELETE FROM `temp_reference_items`";
        if(!query->exec(q)){
            qDebug() << "Sql Error: " << query->lastError().text();
             //return;
        }
        q = QString("INSERT INTO temp_reference_items "
                    "SELECT * FROM items");
        if(ui->type_comboBox->itemData(ui->type_comboBox->currentIndex()).toInt()){
            q.append(QString(" WHERE `type_id`=%1")
                             .arg(ui->type_comboBox->itemData(ui->type_comboBox->currentIndex()).toInt())
                             );
        }

        if(!query->exec(q)){
            qDebug() << "Sql Error: " << query->lastError().text();
            //return;
        }
        model->setTable("temp_reference_items");

        QString f;
        if(!ui->filter_comboBox->currentText().isEmpty()){

            f = (QString("`model` LIKE '%%2%'"
                         " OR `type` LIKE '%%2%'"
                         " OR `name` LIKE '%%2%'"
                         " OR `status` LIKE '%%2%'"
                         " OR `location` LIKE '%%2%'"
                         " OR `IP` LIKE '%%2%'"
                         " OR `model` LIKE '%%2%'"
                         " OR `manufacturer` LIKE '%%2%'"
                         " OR `serialno` LIKE '%%2%'"
                         " OR `number` LIKE '%%2%'"
                         )
                 .arg(ui->type_comboBox->itemData(ui->type_comboBox->currentIndex()).toInt())
                 .arg(ui->filter_comboBox->currentText())
                 );

        }
        qDebug() << "Filter: " << f;
        model->setFilter(f);

        if(!headerView->isRestoredState())
            headerView->setRestoredState(headerView->restoreStateFromSettings(model->tableName()));
        if(qApp->property("marking_rows").toBool()){
            ui->tableView->loadMarked();
        }
        else{
            ui->tableView->cleanMarked();
        }
    }


    model->setRelation(model->fieldIndex("location_id"), QSqlRelation("locations", "id", "location"));
    model->setRelation(model->fieldIndex("type_id"), QSqlRelation("item_types", "id", "type"));
    model->setRelation(model->fieldIndex("operator_id"), QSqlRelation("operators", "id", "name"));
    model->setRelation(model->fieldIndex("status_id"), QSqlRelation("item_status", "id", "status"));

    model->select();
    ui->tableView->setModel(model);
    if(model->lastError().isValid()){
        qDebug() << "Sql Error: " << model->lastError().text();
    }
    if(!qApp->property("show_id").toBool()) ui->tableView->hideColumn(model->fieldIndex("id"));

    model->setHeaderData(model->fieldIndex("value"), Qt::Horizontal, ui->properties_comboBox->currentText(), Qt::EditRole);
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

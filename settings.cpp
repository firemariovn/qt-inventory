#include "settings.h"
#include "ui_settings.h"

#include <QtGui>

Settings::Settings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Settings)
{
    ui->setupUi(this);

    loadAccessibleTables();
    loadSettings();

    connect(ui->selected_pushButton, SIGNAL(clicked()), this, SLOT(setBackgroundBrush()));
    connect(ui->unselected_pushButton, SIGNAL(clicked()), this, SLOT(setBackgroundBrush()));
    connect(ui->marking_bold_checkBox, SIGNAL(toggled(bool)), this, SLOT(setBoldFont(bool)));
}

Settings::~Settings()
{
    delete ui;
}

void Settings::loadAccessibleTables()
{
    QBitArray rights(qApp->property("user_rights").toBitArray());
    QStringList tables;
    if(rights[0]) tables << tr("Users");
    if(rights[3]) tables << tr("Items");
    if(rights[6]) tables << tr("Operators");
    if(rights[9]) tables << tr("Locations");
    if(rights[12]) tables << tr("Departments");
    if(rights[24]) tables << tr("Allocations");
    ui->load_table_comboBox->addItems(tables);
}

void Settings::loadSettings()
{
    //QDir appdir = qApp->applicationDirPath();
    //QSettings settings(appdir.filePath("Inventory.ini"), QSettings::IniFormat);
    QSettings settings(qApp->property("ini").toString(), QSettings::IniFormat);
    settings.beginGroup("InventorySettings");
    settings.beginGroup("Startup");
        ui->load_table_groupBox->setChecked(settings.value("AutostartTable", false).toBool());
        ui->load_table_comboBox->setCurrentIndex(
                    ui->load_table_comboBox->findText(settings.value("AutostartTableName").toString())
                    );
        ui->updates_checkBox->setChecked(settings.value("CheckForUpdates", true).toBool());
    settings.endGroup();//startup
    settings.beginGroup("View");
        ui->show_id_checkBox->setChecked(settings.value("ShowIdEverywhere", false).toBool());
    settings.beginGroup("MarkingRows");
    ui->marking_groupBox->setChecked(settings.value("MarkingEnable", false).toBool());
    ui->marking_bold_checkBox->setChecked(qApp->property("marking_bold_font").toBool());
    QVariant value = qApp->property("marking_selected_brush");
    if (qVariantCanConvert<QBrush>(value)) {
        QBrush brush(qvariant_cast<QBrush>(value));
        ui->selected_pushButton->setPalette(QPalette(brush.color()));
        ui->selected_pushButton->setAutoFillBackground(true);
    }
    value = qApp->property("marking_unselected_brush");
    if (qVariantCanConvert<QBrush>(value)) {
        QBrush brush(qvariant_cast<QBrush>(value));
        ui->unselected_pushButton->setPalette(QPalette(brush.color()));
        ui->unselected_pushButton->setAutoFillBackground(true);
    }
    QFont font = ui->selected_pushButton->font();
    font.setBold(ui->marking_bold_checkBox->isChecked());
    ui->selected_pushButton->setFont(font);
    ui->unselected_pushButton->setFont(font);
    settings.endGroup();//marking rows group
    settings.endGroup();//view group
    settings.endGroup();
}

void Settings::saveSettings()
{
    //QDir appdir = qApp->applicationDirPath();
    //QSettings settings(appdir.filePath("Inventory.ini"), QSettings::IniFormat);
    QSettings settings(qApp->property("ini").toString(), QSettings::IniFormat);
    settings.beginGroup("InventorySettings");
        settings.beginGroup("Startup");
            settings.setValue("AutostartTable", ui->load_table_groupBox->isChecked());
            settings.setValue("AutostartTableName", ui->load_table_comboBox->currentText());
            settings.setValue("CheckForUpdates", ui->updates_checkBox->isChecked());
        settings.endGroup();//startup
        if(qApp->property("show_id").toBool() != ui->show_id_checkBox->isChecked()){
            settings.beginGroup("View");
            settings.setValue("ShowIdEverywhere", ui->show_id_checkBox->isChecked());
            settings.endGroup();//view
            qApp->setProperty("show_id", ui->show_id_checkBox->isChecked());
            settings.beginGroup("TablesHeaders");
            settings.remove("");
            settings.endGroup();//tables headers
            emit showId( ui->show_id_checkBox->isChecked());
        }
        settings.beginGroup("View");
        settings.beginGroup("MarkingRows");
        settings.setValue("MarkingEnable", ui->marking_groupBox->isChecked());
        settings.setValue("MarkingBold", ui->marking_bold_checkBox->isChecked());
        settings.setValue("SelectedBackgroundBrush", ui->selected_pushButton->palette().background());
        settings.setValue("UnselectedBackgroundBrush", ui->unselected_pushButton->palette().background());
        if(qApp->property("marking_rows").toBool() != ui->marking_groupBox->isChecked()){
            qApp->setProperty("marking_rows", ui->marking_groupBox->isChecked());
            emit tableUpdate("Other");
        }
        qApp->setProperty("marking_bold_font", ui->marking_bold_checkBox->isChecked());
        qApp->setProperty("marking_selected_brush", ui->selected_pushButton->palette().background());
        qApp->setProperty("marking_unselected_brush", ui->unselected_pushButton->palette().background());
        settings.endGroup();//marking rows
        settings.endGroup();//view
    settings.endGroup();
}

void Settings::accept()
{
    saveSettings();
    QDialog::accept();
}

void Settings::setBackgroundBrush()
{
    QPushButton* button = qobject_cast<QPushButton *>(sender());
    if(button){
        QColor def = button->palette().background().color();
        if(button == ui->selected_pushButton && def == Qt::white)
            def = palette().color(QPalette::Highlight);
        else if(def == Qt::white) def = Qt::yellow;
        QColor color = QColorDialog::getColor(def, this);
        if (color.isValid()) {
            button->setPalette(QPalette(color));
            button->setAutoFillBackground(true);
        }
    }
}

void Settings::setBoldFont(const bool set)
{
    QFont font = ui->selected_pushButton->font();
    font.setBold(set);
    ui->selected_pushButton->setFont(font);
    ui->unselected_pushButton->setFont(font);
}

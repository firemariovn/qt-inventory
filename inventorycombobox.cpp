#include "inventorycombobox.h"

#include <QtSql>
#include <QtGui>

InventoryComboBox::InventoryComboBox(QWidget *parent) :
    QComboBox(parent)
{
    setInsertPolicy(QComboBox::NoInsert);
}

void InventoryComboBox::showPopup(const int secs)
{
    if(!secs) showComboPopup();
    else QTimer::singleShot(secs*1000, this, SLOT(showComboPopup()));
}

void InventoryComboBox::showComboPopup()
{
    if(isVisible())
        QComboBox::showPopup();
}

QString InventoryComboBox::itemModelText() const
{
    if(model()){
        QSqlTableModel *model = qobject_cast<QSqlTableModel *>(this->model());
        if(model){
            QString field("text");
            if(model->tableName() == "item_types") field = "type";
            else if(model->tableName() == "item_status") field = "status";
            else if(model->tableName() == "locations") field = "location";
            else if(model->tableName() == "operators") field = "name";
            else if(model->tableName() == "departments") field = "department";
            return model->data(model->index(currentIndex(), model->fieldIndex(field))).toString();
        }
    }
    return "";
}

int InventoryComboBox::itemModelId() const
{
    if(model()){
        QSqlTableModel *model = qobject_cast<QSqlTableModel *>(this->model());
        if(model){
            QString field("id");
            return model->data(model->index(currentIndex(), model->fieldIndex(field))).toInt();
        }
    }
    return -1;
}

bool InventoryComboBox::checkData()
{
    if(itemModelId() > -1 && currentText() != "N/A"){
        QPalette ap(qApp->palette());
        setPalette(ap);
        return true;
    }
    else{
        QPalette p(palette());
        p.setColor(QPalette::Base, Qt::yellow);
        setPalette(p);
    }
    return false;
}

void InventoryComboBox::showTip(const QString &msg)
{
    QToolTip::showText(QCursor::pos(), msg);
}

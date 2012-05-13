#include "inventorylineedit.h"

#include <QtGui>

InventoryLineEdit::InventoryLineEdit(QWidget *parent) :
    QLineEdit(parent)
{
}

InventoryLineEdit::InventoryLineEdit(const QString &contents, QWidget *parent) :
    QLineEdit(contents, parent)
{
}

bool InventoryLineEdit::checkData()
{
    if(!text().isEmpty() && text() != "N/A"){
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

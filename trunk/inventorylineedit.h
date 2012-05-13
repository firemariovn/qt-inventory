#ifndef INVENTORYLINEEDIT_H
#define INVENTORYLINEEDIT_H

#include <QLineEdit>

class InventoryLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit InventoryLineEdit(QWidget *parent = 0);
    explicit InventoryLineEdit (const QString& contents, QWidget* parent = 0 );

public slots:
    bool checkData();

};

#endif // INVENTORYLINEEDIT_H

#ifndef INVENTORYCOMBOBOX_H
#define INVENTORYCOMBOBOX_H

#include <QComboBox>

class InventoryComboBox : public QComboBox
{
    Q_OBJECT
public:
    explicit InventoryComboBox(QWidget *parent = 0);

    QString itemModelText() const;
    int itemModelId() const;

public slots:
    void showPopup(const int secs);
    void showTip(const QString& msg);
    bool checkData();

signals:

private slots:
    void showComboPopup();

};

#endif // INVENTORYCOMBOBOX_H

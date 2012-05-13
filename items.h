#ifndef ITEMS_H
#define ITEMS_H

#include <QDialog>


namespace Ui {
    class Items;
}

class QSqlTableModel;
class Logger;
class QLineEdit;

class Items : public QDialog
{
    Q_OBJECT

public:
    explicit Items(QWidget *parent = 0);
    ~Items();

    void accept();
    void setEditMode(const bool edit);
    void loadItem(const int item_id);

    inline void setItemId(const int item_id) { _item_id = item_id; }
    inline int getItemId() const { return _item_id; }
    inline bool isEditMode() const { return edit_mode; }
    inline void setOperatorId(const int operator_id) { _operator_id = operator_id; }
    inline int getOperatorId() const { return _operator_id; }
    inline void setLocationId(const int location_id) { _location_id = location_id; }
    inline int getLocationId() const { return _location_id; }
    inline void setLogger(Logger* l) { logger = l; }

    void setOperatorFilter(const int operator_id);
    void setLocationFilter(const int location_id);
    void setTypeFilter(const int type_id);
    void setStatusFilter(const int status_id);

private:
    void setLineEditCompleter(QLineEdit* lineEdit, const QString& field, const QString& table = "items");
    bool saveItem() const;
    void loadData();
    bool checkNumber() const;

    Ui::Items *ui;
    QSqlTableModel* locations_model;
    QSqlTableModel* operators_model;
    QSqlTableModel* types_model;
    QSqlTableModel* status_model;
    Logger* logger;
    int _item_id;
    int _operator_id;
    int _location_id;
    bool edit_mode;
};

#endif // ITEMS_H

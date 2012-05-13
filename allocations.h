#ifndef ALLOCATIONS_H
#define ALLOCATIONS_H

#include <QDialog>

namespace Ui {
    class Allocations;
}

class QSqlRelationalTableModel;
class QSqlTableModel;
class QModelIndex;
class Logger;

class Allocations : public QDialog
{
    Q_OBJECT

public:
    explicit Allocations(QWidget *parent = 0);
    ~Allocations();

    void loadItems();
    void loadAllocation(const int allocation_id);
    void loadOperators();
    void loadLocations();
    void setItemFilter(const int item_id);
    void setLocationFilter(const int location_id);
    void setOperatorFilter(const int operator_id);
    void setEditMode(const bool edit);

    void accept();

    inline void setAllocationId(const int allocation_id) { _id = allocation_id; }
    inline void setItemId(const int item_id) { _item_id = item_id; }
    inline int getAllocationId() const { return _id; }
    inline int getItemId() const { return _item_id; }
    inline bool isEditMode() const { return edit_mode; }
    inline void setLogger(Logger* l) { logger = l; }

private slots:
    void itemSelected(const QModelIndex& current, const QModelIndex& previous);
    void setItemsFilter(const QString& filter);

private:
    bool saveAllocation();

    Ui::Allocations *ui;
    QSqlRelationalTableModel* items_model;
    QSqlTableModel* locations_model;
    QSqlTableModel* operators_model;
    Logger* logger;
    int _id;
    int _item_id;
    bool edit_mode;
};

#endif // ALLOCATIONS_H

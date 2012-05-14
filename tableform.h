#ifndef TABLEFORM_H
#define TABLEFORM_H

#include <QWidget>
#include <QModelIndex>

class QSqlRelationalTableModel;
//class QModelIndex;

namespace Ui {
    class TableForm;
}

class HeaderView;
class Logger;

class TableForm : public QWidget
{
    Q_OBJECT

public:
    explicit TableForm(QWidget* parent = 0, Qt::WindowFlags f = 0);
    ~TableForm();

    void setTable(const QString& table);
    void setLogger(Logger* l);

signals:
    void tableUpdate(const QString& table);
    void showId(bool show);
    void searchItem(const int item_id, const QString& table);


public slots:
    void addAttachedFile();
    void addItem();
    void needUpdate(const QString& table);
    void showHeaderId(bool show);
    void findItem(const int item_id);

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void addProperty();
    void save();
    void revert();
    void insert();
    void remove();
    void refresh();
    void dataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRigh);
    void fillFilterFields();
    void setFilter(const QString& filter);
    void setFilterCompleter(const QString& field);
    void clearFilter();
    void itemSelected(const QModelIndex& current, const QModelIndex& previous);
    void activatedItem(const int item_id, const QString& table);

    void showDetailedTable(const bool show);
    void showDetailedItems(const bool show);
    void showDetailedAttachments(const bool show);
    void showDetailedRights(const bool show);
    void showTypeProperties(const bool show);
    void showItemProperties(const bool show);
    void updateDetailedView();
    void updateFields();
    void sortIndicatorChanged ( int logicalIndex, Qt::SortOrder order );

    void exportToCsv();

private:
    bool checkUserRights(const int idx, const bool show_message = true) const;
    void setUserRights(const bool has_rights);
    void setDetailedOptions();
    void removeItemType(const QModelIndex& index);
    void removeItem(const QModelIndex& index);
    void removeUser(const QModelIndex& index);
    void removeLocation(const QModelIndex& index);
    void removeItemStatus(const QModelIndex& index);
    void removeOperator(const QModelIndex &index);
    void removeDepartments(const QModelIndex &index);
    void removeLogs(const QList<QModelIndex> &list);
    void editItem(const int id);
    void editAllocation(const int id);
    void editUser(const int id);
    int nullLocationId() const;
    int nullOperatorId() const;
    int nullDepartmentId() const;
    int nullItemId() const;
    int nullStatusId() const;
    int nullTypeId() const;
    QString transFilterField(const QString& field, const bool direction=true) const;
    void saveFilterState();
    void restoreFilterState();

    inline void setCurrentIndex(const QModelIndex& index){ _current = index; }
    inline QModelIndex currentIndex() const { return _current; }

    Ui::TableForm* ui;
    HeaderView* headerView;
    HeaderView* detailedHeaderView;
    QSqlRelationalTableModel* model;
    QSqlRelationalTableModel *detailed_model;
    QAction* show_items;
    QAction* show_attachments;
    QAction* show_rights;
    QAction* show_properties;
    QString detailed_filter;
    Logger* logger;
    QModelIndex _current;
    int current_row;
    int current_id;
    QString last_filter_field;
    QString last_filter;
};

#endif // TABLEFORM_H

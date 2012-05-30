#ifndef INVENTORYTABLEVIEW_H
#define INVENTORYTABLEVIEW_H

#include <QTableView>

class Logger;

class InventoryTableView : public QTableView
{
    Q_OBJECT

public:
    explicit InventoryTableView(QWidget* parent = 0);
    ~InventoryTableView();

    inline void setLogger(Logger* l) { logger = l; }
    void saveMarked();
    void restoreMarked();
    void loadMarked();
    void cleanMarked();


public slots:
    void printPreviewTable();

signals:
    void add_attachment_();
    void add_property_();
    void add_item_();
    void tableUpdate(const QString& table);
    void searchItem(const int item_id, const QString& table);
    void activatedItem(const int item_id, const QString& table);

protected:
    void contextMenuEvent(QContextMenuEvent *event);

private slots:
    void openAttachment();
    void addAttachment();
    void addItem();
    void editItem();
    void copyItem();
    void deleteAttachment();
    void deleteItem();
    void scrapItem();
    void activated(const QModelIndex &index);
    void openRights();
    void allocationItem();
    void findItem();
    void findAllocation();
    void markRow();
    void unmarkRow();
    void markingReverse();
    void hideMarked(bool hide);
    void hideUnmarked(bool hide);
    void printTable(QPrinter* printer);
    void printSelection(QPrinter* printer);
    void savePrinterSettings(QPrinter* printer);
    void loadPrinterSettings(QPrinter* printer);
    void addProperty();
    void editProperty();
    void deleteProperty();

private:
    bool checkUserRights(const int idx, const bool show_message = true) const;

    Logger* logger;

    QAction* add_attachment;
    QAction* open_attachment;
    QAction* delete_attachment;
    QAction* add_item;
    QAction* edit_item;
    QAction* copy_item;
    QAction* scrap_item;
    QAction* allocation;
    QAction* delete_item;
    QAction* open_rights;
    QAction* find_item;
    QAction* find_allocation;
    QAction* mark_row;
    QAction* unmark_row;
    QAction* marking_reverse;
    QAction* hide_marked;
    QAction* hide_unmarked;
    QAction* print;
    QAction* add_property;
    QAction* edit_property;
    QAction* delete_property;

    QStringList marked;
};

#endif // INVENTORYTABLEVIEW_H

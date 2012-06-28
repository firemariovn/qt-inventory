#ifndef ATTACHMENTSVIEW_H
#define ATTACHMENTSVIEW_H

#include <QListView>
#include <QStandardItemModel>

class Logger;

class MyStandardItemModel : public QStandardItemModel
{
    Q_OBJECT
public:
    MyStandardItemModel(QObject *parent);
    Qt::DropActions supportedDropActions() const ;
    QStringList mimeTypes () const ;
    Qt::ItemFlags flags(const QModelIndex &index) const ;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
};

class AttachmentsView : public QListView
{
    Q_OBJECT
public:
    explicit AttachmentsView(QWidget * parent = 0);
    ~AttachmentsView();

protected:
    void contextMenuEvent(QContextMenuEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent* event);
signals:
    
private slots:
    void addAttachments();
    void deleteAttachments();
    void openAttachment();
    void openAttachment(const QModelIndex &index);

private:
    bool checkUserRights(const int idx, const bool show_message = true) const;

    Logger* logger;
    QAction* add_attachment;
    QAction* delete_attachment;
    QAction* open_attachment;
    
};

#endif // ATTACHMENTSVIEW_H

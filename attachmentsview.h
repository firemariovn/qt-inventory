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
    QStringList mimeTypes () const ;
};

class AttachmentsView : public QListView
{
    Q_OBJECT
public:
    explicit AttachmentsView(QWidget * parent = 0);
    ~AttachmentsView();

    inline QList<QString> pendedAttachments() { return pended_attachments; }

protected:
    void contextMenuEvent(QContextMenuEvent *event);
    void dropEvent(QDropEvent* event);
    
private slots:
    void addAttachments();
    void deleteAttachment();
    void openAttachment();
    void openAttachment(const QModelIndex &index);

private:
    bool checkUserRights(const int idx, const bool show_message = true) const;

    Logger* logger;
    QAction* add_attachment;
    QAction* delete_attachment;
    QAction* open_attachment;

    QList<QString> pended_attachments;
    
};

#endif // ATTACHMENTSVIEW_H

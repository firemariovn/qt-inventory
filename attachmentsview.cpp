#include "attachmentsview.h"
#include "logger.h"

#include <QtGui>
#include <QtDebug>

MyStandardItemModel::MyStandardItemModel(QObject *parent) :
    QStandardItemModel(parent)
{
}

Qt::DropActions MyStandardItemModel::supportedDropActions() const
{
    return Qt::IgnoreAction | Qt::CopyAction | Qt::MoveAction;
}

Qt::ItemFlags MyStandardItemModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QStandardItemModel::flags(index);

    if (index.isValid())
        return Qt::ItemIsDropEnabled | defaultFlags;
    else
        return defaultFlags;
}

bool MyStandardItemModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
   qDebug("I am in the model");
   return true;
}

QStringList MyStandardItemModel::mimeTypes() const
 {
     QStringList types;
     types << "text/plain";
     return types;
 }


AttachmentsView::AttachmentsView(QWidget *parent) :
    QListView(parent)
{
    setModel(new MyStandardItemModel(this));

    setAcceptDrops(true);

    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setDragDropMode(QAbstractItemView::DropOnly);

    setDropIndicatorShown(true);

    logger = 0;

    add_attachment = new QAction(QIcon(":/Icons/icons/Attach.png"), tr("Add attachment"), this);
    open_attachment = new QAction(QIcon(":/Icons/icons/Document.png"), tr("Open"), this);
    delete_attachment = new QAction(QIcon(":/Icons/icons/Trash.png"), tr("Delete"), this);

    connect(add_attachment, SIGNAL(triggered()), this, SLOT(addAttachments()));
    connect(delete_attachment, SIGNAL(triggered()), this, SLOT(deleteAttachments()));
    connect(open_attachment, SIGNAL(triggered()), this, SLOT(openAttachment()));

    connect(this, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(openAttachment(QModelIndex)));
}

AttachmentsView::~AttachmentsView()
{
    //if(logger) delete logger; //crash
}

void AttachmentsView::contextMenuEvent(QContextMenuEvent *event)
{
    if(this->model()){
        MyStandardItemModel *model = qobject_cast<MyStandardItemModel *>(this->model());
        QModelIndex index = this->selectionModel()->currentIndex();
        QMenu menu;
        menu.addAction(add_attachment);
        if(index.isValid()){
            menu.addAction(delete_attachment);
            menu.addSeparator();
            menu.addAction(open_attachment);
        }
        menu.exec(event->globalPos());
    }
}

bool AttachmentsView::checkUserRights(const int idx, const bool show_message) const
{
    QBitArray rights(qApp->property("user_rights").toBitArray());
    if(!rights[idx]){
        if(show_message){
        QMessageBox msgBox;
            msgBox.setText(tr("You are not authorized to perform this operation!\nPlease contact your system administrator."));
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.exec();
        }
        if(logger) logger->writeLog(Logger::Info, Logger::Other, QString("Attempt to commit the procedure %1 without any rights")
                                    .arg(idx)
                                    );
        return false;
    }
    else {
        return true;
    }
}

void AttachmentsView::addAttachments()
{
    if(!this->checkUserRights(16)) return;
    MyStandardItemModel *model = qobject_cast<MyStandardItemModel *>(this->model());
    if(model){
        QStringList files = QFileDialog::getOpenFileNames(
                                 this,
                                 "Select one or more files to open",
                                  qApp->property("last_attached_dir").toString());
        QStringList::Iterator it = files.begin();
        while(it != files.end()) {
            QFileInfo fi(*it);
            QStandardItem* item = new QStandardItem(fi.fileName());

            item->setEditable(false);
            item->setData(0, Qt::UserRole+1);//attachment id
            item->setData(0, Qt::UserRole+2);//item_id
            item->setData(*it, Qt::UserRole+3);//path

            item->setToolTip(item->data(Qt::UserRole+3).toString());//tool tip

            model->setItem(model->rowCount(), 0, item);

            ++it;
        }
    }
}

void AttachmentsView::deleteAttachments()
{
    MyStandardItemModel *model = qobject_cast<MyStandardItemModel *>(this->model());
    if(model){
        QModelIndexList list = this->selectedIndexes();
        for (int i = 0; i < list.size(); ++i) {
            model->removeRow(model->itemFromIndex(list.at(i))->row());
         }
    }
}

void AttachmentsView::openAttachment()
{
    openAttachment(this->currentIndex());
}

void AttachmentsView::openAttachment(const QModelIndex & index )
{
    MyStandardItemModel *model = qobject_cast<MyStandardItemModel *>(this->model());
    if(model && index.isValid()){
        QUrl url;
        QString filePath = model->itemFromIndex(index)->data(Qt::UserRole+3).toString();
        if (filePath.startsWith("\\\\") || filePath.startsWith("//"))
            url.setUrl(QDir::toNativeSeparators(filePath));
        else
            url = QUrl::fromLocalFile(filePath);
        if(!QDesktopServices::openUrl(url)){
            if(logger) logger->writeLog(Logger::Error, Logger::Attachments,
                                        tr("Error with document opening. The url path:\n%1")
                                        .arg(filePath)
                                        );
        }
        else{
            if(logger) logger->writeLog(Logger::View, Logger::Attachments, tr("Open attachment: %1")
                                        .arg(filePath));
        }
    }
}

void AttachmentsView::dropEvent(QDropEvent *event)
{
    qDebug() << "Drop event";
    event->acceptProposedAction();
}

void AttachmentsView::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

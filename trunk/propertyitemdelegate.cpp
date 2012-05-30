#include "propertyitemdelegate.h"

#include <QLineEdit>
#include <QCompleter>

PropertyItemDelegate::PropertyItemDelegate(QObject *parent) :
    QItemDelegate(parent)
{
}

void PropertyItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QLineEdit* lineedit = static_cast<QLineEdit*>(editor);
    if(lineedit){
        QStringList list = index.model()->data(index, Qt::UserRole).toStringList();
        QCompleter* completer = new QCompleter(list, lineedit);
        lineedit->setCompleter(completer);
    }
    QItemDelegate::setEditorData(lineedit, index);
}

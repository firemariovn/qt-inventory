#include "propertyitemdelegate.h"

#include <QComboBox>
#include <QCompleter>
#include <QRect>
#include <QtDebug>

PropertyItemDelegate::PropertyItemDelegate(QObject *parent) :
    QItemDelegate(parent)
{
}

QWidget* PropertyItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QComboBox* editor = new QComboBox(parent);
    editor->setEditable(true);
    editor->setInsertPolicy(QComboBox::NoInsert);
    QStringList list = index.model()->data(index, Qt::UserRole).toStringList();
    editor->addItems(list);
    QCompleter* completer = new QCompleter(list, editor);
    editor->setCompleter(completer);
    return editor;
}

void PropertyItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QString value = index.model()->data(index, Qt::EditRole).toString();

    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    if(comboBox){
        comboBox->setEditText(value);
    }
    else QItemDelegate::setEditorData(editor, index);
}

void PropertyItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    if(comboBox){
        QString value = comboBox->currentText();
        model->setData(index, value, Qt::EditRole);
    }
    else QItemDelegate::setModelData(editor, model, index);
}

void PropertyItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QRect rect;
    rect.setTopLeft(option.rect.topLeft());
    rect.setBottomRight(option.rect.bottomRight());
    rect.setHeight(option.fontMetrics.height()+4);
    editor->setGeometry(rect);
}


#ifndef CHECKDELEGATE_H
#define CHECKDELEGATE_H

#include <QSqlRelationalDelegate>

class CheckDelegate : public QSqlRelationalDelegate
{
    Q_OBJECT
public:
    explicit CheckDelegate(QObject *parent = 0);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
                   const QModelIndex &index) const;

protected:
    void drawBackground(QPainter *painter, const QStyleOptionViewItem &option,
                            const QModelIndex &index) const;

};

#endif // CHECKDELEGATE_H

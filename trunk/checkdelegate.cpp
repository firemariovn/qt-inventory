#include "checkdelegate.h"

#include <QtGui>

CheckDelegate::CheckDelegate(QObject *parent) : QSqlRelationalDelegate(parent)
{
}

void CheckDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                           const QModelIndex &index) const
 {

        QStyleOptionViewItem opt(option);
        //opt.palette.setBrush(QPalette::Highlight, qApp->property("marking_selected_brush"));
        QVariant value = qApp->property("marking_selected_brush");
        if (qVariantCanConvert<QBrush>(value)) {
            opt.palette.setBrush(QPalette::Highlight, qvariant_cast<QBrush>(value));
        }
        value = qApp->property("marking_unselected_brush");
        if (qVariantCanConvert<QBrush>(value)) {
            QBrush brush(qvariant_cast<QBrush>(value));
            if(brush.color() != Qt::white){
                drawBackground(painter, opt, index);
            }
        }
        opt.font.setBold(qApp->property("marking_bold_font").toBool());
        QSqlRelationalDelegate::paint(painter, opt, index);
 }

void CheckDelegate::drawBackground(QPainter *painter,
                                   const QStyleOptionViewItem &option,
                                   const QModelIndex &index) const
{
    if (!(option.showDecorationSelected && (option.state & QStyle::State_Selected))) {
        QVariant value = qApp->property("marking_unselected_brush");
        if (qVariantCanConvert<QBrush>(value)) {
            QPointF oldBO = painter->brushOrigin();
            painter->setBrushOrigin(option.rect.topLeft());
            painter->fillRect(option.rect, qvariant_cast<QBrush>(value));
            painter->setBrushOrigin(oldBO);
        }
    }
}




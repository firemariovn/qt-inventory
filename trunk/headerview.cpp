#include "headerview.h"

#include <QtGui>
#include <QtSql>

HeaderView::HeaderView(Qt::Orientation orientation, QWidget * parent) :
    QHeaderView(orientation, parent)
{
    signalMapper = new QSignalMapper(this);

    setMovable(true);
    setClickable(true);

    restoredState = false;

    connect(signalMapper, SIGNAL(mapped(int)), this, SIGNAL(clickedMenu(int)));
    connect(this, SIGNAL(clickedMenu(int)), this, SLOT(contextMenuActivated(int)));
}

HeaderView::~HeaderView()
{
}

void HeaderView::contextMenuEvent(QContextMenuEvent *event)
{
    if(this->count()){
        QStringList disabledFields;
        disabledFields << "password" << "deleted" << "item_id" << "tablename" << "allocations_data";
        if(!qApp->property("show_id").toBool()) disabledFields << "id";
        QMenu menu;
        QAbstractItemModel* model = this->model();
        //QSqlRelationalTableModel *model = qobject_cast<QSqlRelationalTableModel *>(this->model());
        for(int n=0; n<this->count(); n++){
            QString field = model->headerData(n, Qt::Horizontal).toString();
            if(!disabledFields.contains(field)){
                QAction* action = new QAction(field, this);
                action->setCheckable(true);
                action->setChecked(!isSectionHidden(n));
                connect(action, SIGNAL(triggered()), signalMapper, SLOT(map()));
                signalMapper->setMapping(action, n);
                menu.addAction(action);
            }
        }
        menu.exec(event->globalPos());
    }
}

void HeaderView::contextMenuActivated(const int section)
{
    if(isSectionHidden(section)) showSection(section);
    else{
        if(hiddenSectionCount() < (count()-1)) hideSection(section);
    }
    emit updateFields();
}

void HeaderView::saveStateToSettings(const QString& table)
{
    //QDir appdir = qApp->applicationDirPath();
    //QSettings settings(appdir.filePath("Inventory.ini"), QSettings::IniFormat);
    QSettings settings(qApp->property("ini").toString(), QSettings::IniFormat);
    settings.beginGroup("TablesHeaders");
    settings.setValue(QString("%1_%2").arg(qApp->property("database").toString()).arg(table), this->saveState());
    settings.endGroup();
}

bool HeaderView::restoreStateFromSettings(const QString& table)
{
    //QDir appdir = qApp->applicationDirPath();
    //QSettings settings(appdir.filePath("Inventory.ini"), QSettings::IniFormat);
    QSettings settings(qApp->property("ini").toString(), QSettings::IniFormat);
    settings.beginGroup("TablesHeaders");
    bool ok = this->restoreState(settings.value(QString("%1_%2").arg(qApp->property("database").toString()).arg(table)).toByteArray());
    settings.endGroup();
    return ok;
}

void HeaderView::showId(bool show)
{
    QSqlRelationalTableModel *model = qobject_cast<QSqlRelationalTableModel *>(this->model());
    if(!model) return;
    int section = model->fieldIndex("id");
    if(section >= 0){
        show ? showSection(section) : hideSection(section);
        saveStateToSettings(model->tableName());
    }
}

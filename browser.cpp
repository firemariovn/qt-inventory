/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the demonstration applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "browser.h"
#include "headerview.h"

#include <QtGui>
#include <QtSql>

Browser::Browser(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);

    table->addAction(insertRowAction);
    table->addAction(deleteRowAction);
    table->addAction(actionPrint);

    HeaderView* headerView = new HeaderView(Qt::Horizontal);
    table->setHorizontalHeader(headerView);

    if (QSqlDatabase::drivers().isEmpty())
        QMessageBox::information(this, tr("No database drivers found"),
                                 tr("This browser requires at least one Qt database driver. "
                                    "Please check the documentation how to build the "
                                    "Qt SQL plugins."));

    updateActions();
    fillSavedViews();
    emit statusMessage(tr("Ready."));
}

Browser::~Browser()
{
}

void Browser::exec()
{
    QSqlQueryModel *model = new QSqlQueryModel(table);
    model->setQuery(QSqlQuery(sqlEdit->toPlainText(), connectionWidget->currentDatabase()));
    table->setModel(model);

    if (model->lastError().type() != QSqlError::NoError)
        emit statusMessage(model->lastError().text());
    else if (model->query().isSelect())
        emit statusMessage(tr("Query OK."));
    else
        emit statusMessage(tr("Query OK, number of affected rows: %1").arg(
                           model->query().numRowsAffected()));

    updateActions();
}


void Browser::addConnection()
{
    connectionWidget->refresh();
}


void Browser::showTable(const QString &t)
{
    QSqlTableModel *model = new QSqlTableModel(table, connectionWidget->currentDatabase());
    model->setEditStrategy(QSqlTableModel::OnRowChange);
    model->setTable(connectionWidget->currentDatabase().driver()->escapeIdentifier(t, QSqlDriver::TableName));
    model->select();
    if (model->lastError().type() != QSqlError::NoError)
        emit statusMessage(model->lastError().text());
    table->setModel(model);
    if(edit_checkBox->isChecked()){
        table->setEditTriggers(QAbstractItemView::DoubleClicked|QAbstractItemView::EditKeyPressed);
    }
    else    table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    connect(table->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
            this, SLOT(currentChanged()));
    updateActions();
}

void Browser::showMetaData(const QString &t)
{
    QSqlRecord rec = connectionWidget->currentDatabase().record(t);
    QStandardItemModel *model = new QStandardItemModel(table);

    model->insertRows(0, rec.count());
    model->insertColumns(0, 7);

    model->setHeaderData(0, Qt::Horizontal, "Fieldname");
    model->setHeaderData(1, Qt::Horizontal, "Type");
    model->setHeaderData(2, Qt::Horizontal, "Length");
    model->setHeaderData(3, Qt::Horizontal, "Precision");
    model->setHeaderData(4, Qt::Horizontal, "Required");
    model->setHeaderData(5, Qt::Horizontal, "AutoValue");
    model->setHeaderData(6, Qt::Horizontal, "DefaultValue");


    for (int i = 0; i < rec.count(); ++i) {
        QSqlField fld = rec.field(i);
        model->setData(model->index(i, 0), fld.name());
        model->setData(model->index(i, 1), fld.typeID() == -1
                ? QString(QVariant::typeToName(fld.type()))
                : QString("%1 (%2)").arg(QVariant::typeToName(fld.type())).arg(fld.typeID()));
        model->setData(model->index(i, 2), fld.length());
        model->setData(model->index(i, 3), fld.precision());
        model->setData(model->index(i, 4), fld.requiredStatus() == -1 ? QVariant("?")
                : QVariant(bool(fld.requiredStatus())));
        model->setData(model->index(i, 5), fld.isAutoValue());
        model->setData(model->index(i, 6), fld.defaultValue());
    }
    table->setModel(model);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    updateActions();
}

void Browser::insertRow()
{
    QSqlTableModel *model = qobject_cast<QSqlTableModel *>(table->model());
    if (!model)
        return;

    QModelIndex insertIndex = table->currentIndex();
    int row = insertIndex.row() == -1 ? 0 : insertIndex.row();
    model->insertRow(row);
    insertIndex = model->index(row, 0);
    table->setCurrentIndex(insertIndex);
    table->edit(insertIndex);
}

void Browser::deleteRow()
{
    QSqlTableModel *model = qobject_cast<QSqlTableModel *>(table->model());
    if (!model)
        return;

    model->setEditStrategy(QSqlTableModel::OnManualSubmit);

    QModelIndexList currentSelection = table->selectionModel()->selectedIndexes();
    for (int i = 0; i < currentSelection.count(); ++i) {
        if (currentSelection.at(i).column() != 0)
            continue;
        model->removeRow(currentSelection.at(i).row());
    }

    model->submitAll();
    model->setEditStrategy(QSqlTableModel::OnRowChange);

    updateActions();
}

void Browser::updateActions()
{
    bool enablePrint = qobject_cast<QSqlTableModel *>(table->model());
    bool enableIns = edit_checkBox->isChecked() && qobject_cast<QSqlTableModel *>(table->model());
    bool enableDel = enableIns && table->currentIndex().isValid();

    actionPrint->setEnabled(enablePrint);
    insertRowAction->setEnabled(enableIns);
    deleteRowAction->setEnabled(enableDel);

    groupBox->setHidden(!edit_checkBox->isChecked());

    setWindowTitle(tr("SQL Browser - %1")
                   .arg(edit_checkBox->isChecked() ? tr("writable") : tr("read only"))
                   );
}

void Browser::editDirectly(bool edit)
{
    if(edit){
        if(!this->checkUserRights(30)){
            edit_checkBox->setChecked(false);
            return;
        }
        int ret = QMessageBox::warning(0, tr("Inventory"),
                                        tr("Direct editing and SQL is a powerful tool, "
                                       "but incorrect use may result to serious data corruption.\n"
                                       "SQL Browser should be used very carefully at your own risk!\n"
                                       "Do you want to continue?\n"),
                                        QMessageBox::Ok | QMessageBox::Cancel);
        if(ret == QMessageBox::Cancel){
            edit_checkBox->setChecked(false);
        }
    }
    QSqlTableModel *model = qobject_cast<QSqlTableModel *>(table->model());
    if(model){
        showTable(model->tableName());
    }
    else updateActions();
}

void Browser::printPreviewTable()
{
    #ifndef QT_NO_PRINTER
    QSqlTableModel *model = qobject_cast<QSqlTableModel *>(table->model());
    if (!model ) return;
    QPrinter* printer = new QPrinter();
    //this->loadPrinterSettings(printer);
    QPrintPreviewDialog* preview = new QPrintPreviewDialog(printer, this, Qt::WindowMinMaxButtonsHint);

    connect(preview, SIGNAL(paintRequested(QPrinter*)), this, SLOT(printTable(QPrinter*)));
    preview->setWindowTitle(tr("Print preview: SQL Browser"));
    preview->exec();
    //this->savePrinterSettings(printer);

    #endif
}

void Browser::printTable(QPrinter* printer)
/* copyright Михаил Филоненко - Russian Qt Forum */
{
    #ifndef QT_NO_PRINTER

    QSqlTableModel *model = qobject_cast<QSqlTableModel *>(table->model());
    if (!model ) return;

    QHeaderView* hHeader = table->horizontalHeader();
    QHeaderView* vHeader = table->verticalHeader();

    QPainter painter(printer);
    // Check that there is a valid device to print to.
    if (!painter.isActive()) return;
    int pageLeft = 10;
    int pageTop = 10;
    int pageHeight = painter.device()->height();
    int pageWidth = painter.device()->width();
    int x = pageLeft;
    int y = pageTop;
    QList<QVector<QLine> > linePages;
    QVector<QLine> verticalLines;
    QList<QList<int> > sectionPages;
    QList<int> sections;
    QList<QList<int> > widthPages;
    QList<int> widths;
    QList<int> pageRights;
    QList<QStringList> pageLabels;
    QStringList labels;
    int logicalIndex;

    // Перечисляем столбцы
    for (int i = 0; i < hHeader->count(); ++i) {
        logicalIndex = hHeader->logicalIndex(i);
        // Если столбец не скрыт
        if (!hHeader->isSectionHidden(logicalIndex)) {
        // Если уже не помещаемся на страницу пошем новую страницу
            if (pageWidth <= x + hHeader->sectionSize(logicalIndex)) {
                verticalLines.append(QLine(x, pageTop, x, pageHeight));
                pageRights.append(x);
                x = pageLeft;
                sectionPages.append(sections);
                sections.clear();
                linePages.append(verticalLines);
                verticalLines.clear();
                widthPages.append(widths);
                widths.clear();
                pageLabels.append(labels);
                labels.clear();
            }
            // Добавляем, так как колонка видимая
            sections.append(logicalIndex);
            widths.append(hHeader->sectionSize(logicalIndex));
            labels.append(model->headerData(logicalIndex, Qt::Horizontal).toString());
            verticalLines.append(QLine(x, pageTop, x, pageHeight));
            // Смещаемся правее на ширину колонки
            x += hHeader->sectionSize(logicalIndex);
        }
    }

    if (!sections.isEmpty()) {
        verticalLines.append(QLine(x, pageTop, x, pageHeight));
        pageRights.append(x);
        x = pageLeft;
        sectionPages.append(sections);
        sections.clear();
        linePages.append(verticalLines);
        verticalLines.clear();
        widthPages.append(widths);
        widths.clear();
        pageLabels.append(labels);
        labels.clear();
    }

    int columnPageCount = sectionPages.count();
    int columnCount = sections.count();
    int rowCount = model->rowCount(table->rootIndex());
    int rowHeight;
    int columnWidth;
    QModelIndex currentIndex;
    int row = 0;
    int lastPageRow = 0;
    int columnPageIndex = 0;
    int rightColumn;

    while (true) {
        // Получаем колонки текущей страницы
        sections = sectionPages.at(columnPageIndex);
        widths = widthPages.at(columnPageIndex);
        verticalLines = linePages.at(columnPageIndex);
        // Количество колонок текущей страницы
        columnCount = sections.count();
        rightColumn = pageRights.at(columnPageIndex);
        labels = pageLabels.at(columnPageIndex);
        // Перемещаемся в левый верхний угол страницы
        x = pageLeft;
        y = pageTop;
        rowHeight = hHeader->height();

        // Рисуем колонки
        for (int i = 0; i < columnCount; ++i) {
            // Текущее значение
            columnWidth = widths.at(i);
            painter.drawText(x, y, columnWidth, rowHeight
                             , currentIndex.data(Qt::TextAlignmentRole).toInt()
                               | Qt::TextWordWrap
                               | Qt::AlignVCenter
                               | Qt::AlignHCenter
                               , labels.at(i)
                              );
            painter.drawLine(pageLeft, y, rightColumn, y);
            // Перемащаемся правее
            x += widths.at(i);
         }

         y += rowHeight;
         x = pageLeft;

         // Пока можем рисовать на текущей странице
         while (true) {
            // Высота текущей строки
            rowHeight = vHeader->sectionSize(row);

            if (y + rowHeight > pageHeight) {
                //--row;
                break;
            }

            // Рисуем колонки
            for (int i = 0; i < columnCount; ++i) {
                // Текущее значение
                currentIndex = model->index(row, sections.at(i), table->rootIndex());
                columnWidth = widths.at(i);

                if (!currentIndex.data().isNull())
                    painter.drawText(x, y, columnWidth, rowHeight
                                   , currentIndex.data(Qt::TextAlignmentRole).toInt()
                                   | Qt::TextWordWrap
                                   | Qt::AlignVCenter
                                   | Qt::AlignHCenter
                                   , currentIndex.data().toString()
                                  );

                 painter.drawLine(pageLeft, y, rightColumn, y);
                 // Перемащаемся правее
                 x += widths.at(i);
              }

              x = pageLeft;
              // Перемещаемся ниже
              y += rowHeight;
              // Берем следующую строку
              ++row;

              // Если строки кончились
              if (row >= rowCount)
                break;
            }

            // Рисуем завершающую горизонтальную линию
            painter.drawLine(pageLeft, y, rightColumn, y);
            // Рисуем вертикальные линии
            painter.drawLines(verticalLines);
            // Вытираем лишние концы
            // TODO: hard-coded background color for page
            painter.fillRect(pageLeft, y + 1, pageWidth, pageHeight - (y + 1), QBrush(Qt::white));

            // Если предыдущие колонки не поместились
            if (columnPageIndex < columnPageCount - 1) {
                ++columnPageIndex;
                row = lastPageRow;
            // Предыдущие колонки поместились
            } else {
                columnPageIndex = 0;
                lastPageRow = row;

                // Строки закончились
                if (row >= rowCount)
                    break;
            }

            // Создаем новую страницу
            printer->newPage();
    }
    #endif
}

void Browser::fillSavedViews()
{
    QSqlQueryModel *model = new QSqlQueryModel;
    model->setQuery("SELECT name, sql FROM sqlite_master WHERE type='view'");

    views_listView->setModel(model);
}

void Browser::viewActivated(const QModelIndex &index)
{
    QSqlQueryModel *views_model = qobject_cast<QSqlQueryModel *>(views_listView->model());
    if(!views_model) return;
    sqlEdit->setPlainText((views_model->data(views_model->index(index.row(), views_model->record(index.row()).indexOf("sql"))).toString())
                          .remove(QString("CREATE VIEW '%1' AS ").arg(index.data().toString()))
                          );

    QString query(QString("SELECT * FROM '%1'").arg(index.data().toString()));
    QSqlQueryModel* model = new QSqlQueryModel(table);
    model->setQuery(QSqlQuery(query, connectionWidget->currentDatabase()));
    table->setModel(model);

    if (model->lastError().type() != QSqlError::NoError)
        emit statusMessage(model->lastError().text());
    else if (model->query().isSelect())
        emit statusMessage(tr("Query OK."));
    else
        emit statusMessage(tr("Query OK, number of affected rows: %1").arg(
                           model->query().numRowsAffected()));

    updateActions();
}

void Browser::addView()
{
    if(sqlEdit->toPlainText().isEmpty()) return;
    bool ok;
    QString text = QInputDialog::getText(this, tr("SQL Browser"),
                                              tr("Saved query name:"), QLineEdit::Normal,
                                              "", &ok);
    if (ok && !text.isEmpty()){
        QSqlQuery* query = new QSqlQuery();
        if(!query->exec(QString("CREATE VIEW '%1' AS %2")
                        .arg(text)
                        .arg(sqlEdit->toPlainText())
                        )){
            emit statusMessage(query->lastError().text());
        }
        else{
            emit statusMessage(tr("Query OK."));
            fillSavedViews();
        }
    }
}

void Browser::removeView()
{
    if(views_listView->currentIndex().isValid()){
        int ret = QMessageBox::question(this, tr("SQL Browser"),
                tr("Do you want to delete %1?\nThis could not be restored!")
                                    .arg(views_listView->currentIndex().data().toString()),
                                    QMessageBox::Ok | QMessageBox::Cancel);
        if(ret == QMessageBox::Cancel) return;
        QSqlQuery* query = new QSqlQuery();
        if(!query->exec(QString("DROP VIEW '%1'")
                        .arg(views_listView->currentIndex().data().toString())
                        )){
            emit statusMessage(query->lastError().text());
        }
        else{
            emit statusMessage(tr("Query OK."));
            fillSavedViews();
        }
    }
}

void Browser::startView()
{
    if(views_listView->currentIndex().isValid()){
        viewActivated(views_listView->currentIndex());
    }
}

bool Browser::checkUserRights(const int idx, const bool show_message) const
{
    QBitArray rights(qApp->property("user_rights").toBitArray());
    if(!rights[idx]){
        if(show_message){
        QMessageBox msgBox;
            msgBox.setText(tr("You are not authorized to perform this operation!\nPlease contact your system administrator."));
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.exec();
        }
        return false;
    }
    else {
        return true;
    }
}

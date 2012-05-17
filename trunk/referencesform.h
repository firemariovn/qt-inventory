#ifndef REFERENCESFORM_H
#define REFERENCESFORM_H

#include <QWidget>

namespace Ui {
class ReferencesForm;
}

class HeaderView;
class QSqlRelationalTableModel;

class ReferencesForm : public QWidget
{
    Q_OBJECT
    
public:
    explicit ReferencesForm(QWidget *parent = 0);
    ~ReferencesForm();

signals:
    void searchItem(const int item_id, const QString& table);

private slots:
    void fillProperties(const int index);
    void fillFilter(const int index);
    void setFilter();
    
private:
    void fillTypes();


    Ui::ReferencesForm *ui;
    HeaderView* headerView;
    QSqlRelationalTableModel* model;
};

#endif // REFERENCESFORM_H

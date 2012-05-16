#ifndef REFERENCESFORM_H
#define REFERENCESFORM_H

#include <QWidget>

namespace Ui {
class ReferencesForm;
}

class ReferencesForm : public QWidget
{
    Q_OBJECT
    
public:
    explicit ReferencesForm(QWidget *parent = 0);
    ~ReferencesForm();

private slots:
    void fillProperties(const int type_id = 0);
    
private:
    void fillFilters();


    Ui::ReferencesForm *ui;
};

#endif // REFERENCESFORM_H

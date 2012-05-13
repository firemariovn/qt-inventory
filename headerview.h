#ifndef HEADERVIEW_H
#define HEADERVIEW_H

#include <QHeaderView>

class QSignalMapper;

class HeaderView : public QHeaderView
{
    Q_OBJECT
public:
    explicit HeaderView(Qt::Orientation orientation, QWidget * parent = 0);
    ~HeaderView();


    bool restoreStateFromSettings(const QString& table);
    void saveStateToSettings(const QString& table);
    inline bool isRestoredState() const { return restoredState; }
    inline void setRestoredState(bool restored) { restoredState = restored; }

public slots:
    void showId(bool show);

protected:
    void contextMenuEvent(QContextMenuEvent *event);

signals:
    void clickedMenu(const int section);
    void updateFields();

private slots:
    void contextMenuActivated(const int section);

private:


    QSignalMapper *signalMapper;
    QString table_name;
    bool restoredState;

};

#endif // HEADERVIEW_H

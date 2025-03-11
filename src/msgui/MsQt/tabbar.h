#ifndef TABBAR_H
#define TABBAR_H

#include <QWidget>
#include <QMap>
#include <QButtonGroup>
#include "flowlayout.h"
#include "mypushbutton.h"

class TabBar : public QWidget
{
    Q_OBJECT
public:
    explicit TabBar(QWidget *parent = nullptr);

    void addTab(const QString &str, int index = -1);
    QString tabText(int index) const;
    void setTabText(int index, const QString &str);

    void clear();

    void hideTab(int index);
    void showTab(int index);

    void editCurrentTab(int index);
    void setCurrentTab(int index);
    void editFirstTab();
    void setFirstTab();
    int currentTab();
    void setHSpacing(int hSpacing);
    void setLeftMargin(int leftMargin);
    int tabCount();

protected:
    void paintEvent(QPaintEvent *event) override;

signals:
    void tabClicked(int index);

public slots:

private slots:
    void onButtonGroupClicked(int id);

private:
    FlowLayout *m_flowLayout = nullptr;
    QMap<int, QPushButton *> m_buttonMap;
    QButtonGroup *m_buttonGroup = nullptr;
};

#endif // TABBAR_H

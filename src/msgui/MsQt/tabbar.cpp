#include "tabbar.h"
#include <QPainter>

TabBar::TabBar(QWidget *parent)
    : QWidget(parent)
{
    m_flowLayout = new FlowLayout(this, 0, 60, -1);
    m_flowLayout->setContentsMargins(-1, 0, -1, 0);
    setLayout(m_flowLayout);

    m_buttonGroup = new QButtonGroup(this);
    connect(m_buttonGroup, SIGNAL(buttonClicked(int)), this, SLOT(onButtonGroupClicked(int)));
}

void TabBar::addTab(const QString &str, int index)
{
    MyPushButton *button = new MyPushButton(str, this);
    button->setCheckable(true);
    button->setMinimumSize(178, 35);
    button->setStyleSheet(
        "QPushButton\
        {\
            border: 0px;\
            border-radius: 0px;\
            color: #4a4a4a;\
            background-color: transparent;\
            border-bottom: 1px solid #CDCDCD;\
        }\
        QPushButton:hover\
        {\
            color: #0082b3;\
            background-color: #e1e1e1;\
            border-bottom:1px solid #0082b3;\
        }\
        QPushButton:checked\
        {\
            color: #ffffff;\
            background-color: #09A8E2;\
            border-bottom:1px solid #0082b3;\
        }");

    if (index == -1) {
        index = m_buttonMap.size();
    }
    m_buttonGroup->addButton(button, index);
    m_buttonMap.insert(index, button);
    m_flowLayout->addWidget(button);
}

QString TabBar::tabText(int index) const
{
    if (m_buttonMap.contains(index)) {
        return m_buttonMap.value(index)->text();
    }
    return QString();
}

void TabBar::setTabText(int index, const QString &str)
{
    if (m_buttonMap.contains(index)) {
        m_buttonMap.value(index)->setText(str);
    }
}

void TabBar::clear()
{
    for (auto iter = m_buttonMap.constBegin(); iter != m_buttonMap.constEnd(); ++iter) {
        QPushButton *button = iter.value();
        m_buttonGroup->removeButton(button);
        m_flowLayout->removeWidget(button);
        button->deleteLater();
    }
    m_buttonMap.clear();
}

void TabBar::hideTab(int index)
{
    if (m_buttonMap.contains(index)) {
        m_buttonMap.value(index)->setVisible(false);
    }
}

void TabBar::showTab(int index)
{
    if (m_buttonMap.contains(index)) {
        m_buttonMap.value(index)->setVisible(true);
    }
}

void TabBar::editCurrentTab(int index)
{
    if (m_buttonMap.contains(index)) {
        m_buttonMap.value(index)->setChecked(true);
    }
}

void TabBar::setCurrentTab(int index)
{
    if (m_buttonMap.contains(index)) {
        m_buttonMap.value(index)->setChecked(true);
        onButtonGroupClicked(index);
    }
}

void TabBar::editFirstTab()
{
    for (auto iter = m_buttonMap.constBegin(); iter != m_buttonMap.constEnd(); ++iter) {
        auto button = iter.value();
        if (button->isVisible()) {
            button->setChecked(true);
            break;
        }
    }
}

void TabBar::setFirstTab()
{
    if (!m_buttonMap.isEmpty()) {
        auto iter = m_buttonMap.constBegin();
        int index = iter.key();
        QPushButton *button = iter.value();
        button->setChecked(true);
        onButtonGroupClicked(index);
    }
}

int TabBar::currentTab()
{
    return m_buttonGroup->checkedId();
}

void TabBar::setHSpacing(int hSpacing)
{
    delete this->layout();
    m_flowLayout = new FlowLayout(this, 0, hSpacing, -1);
    m_flowLayout->setContentsMargins(-1, 0, -1, 0);
    setLayout(m_flowLayout);
}

void TabBar::setLeftMargin(int leftMargin)
{
    int hSpacing = m_flowLayout->horizontalSpacing();
    delete this->layout();
    m_flowLayout = new FlowLayout(this, 0, hSpacing, -1);
    m_flowLayout->setContentsMargins(leftMargin, 0, -1, 0);
    setLayout(m_flowLayout);
}

void TabBar::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
}

void TabBar::onButtonGroupClicked(int id)
{
    emit tabClicked(id);
}

int TabBar::tabCount() {
    return m_buttonMap.size();
}
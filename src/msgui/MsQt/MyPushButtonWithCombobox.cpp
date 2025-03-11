#include "MyPushButtonWithCombobox.h"
#include "MyDebug.h"
#include <QAction>
#include <QApplication>
#include <QHBoxLayout>
#include <QListWidget>

MyPushButtonWithCombobox::MyPushButtonWithCombobox(QWidget *parent)
    : MyPushButton(parent)
{
    m_listWidget = new MyListWidget(this);

    connect(this, SIGNAL(clicked(bool)), this, SLOT(onButtonClicked(bool)));
    connect(m_listWidget, SIGNAL(itemClicked(QString)), this, SLOT(onItemClicked(QString)));

    QLabel *iconLabel = new QLabel(this);
    m_textLabel = new QLabel(this);
    iconLabel->setStyleSheet("background: transparent;");
    m_textLabel->setStyleSheet("background: transparent;");
    iconLabel->setPixmap(QPixmap(":/common/common/drop-down-black.png"));
    m_textLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    QHBoxLayout *myLayout = new QHBoxLayout();
    myLayout->addSpacerItem(new QSpacerItem(38, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
    myLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
    myLayout->addWidget(m_textLabel);
    myLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
    myLayout->addWidget(iconLabel);
    myLayout->setStretch(0, 1);
    myLayout->setStretch(1, 1);
    myLayout->setStretch(2, 0);
    myLayout->setStretch(3, 1);
    myLayout->setStretch(4, 0);
    myLayout->setMargin(0);
    myLayout->setSpacing(0);
    setLayout(myLayout);
}

void MyPushButtonWithCombobox::setCurrentIndex(int index)
{
    m_listWidget->setCurrentIndex(index);
}

void MyPushButtonWithCombobox::addItem(QString text, int data)
{
    m_listWidget->addItem(text, data);
}

void MyPushButtonWithCombobox::clear()
{
    m_listWidget->clear();
}

int MyPushButtonWithCombobox::currentInt()
{
    return m_listWidget->currentInt();
}

void MyPushButtonWithCombobox::onButtonClicked(bool clicked)
{
    Q_UNUSED(clicked);
    m_listWidget->show();
    QPoint p = mapToGlobal(QPoint(0, 0));
    p.setY(p.y() + 30);
    m_listWidget->move(p);
}

void MyPushButtonWithCombobox::onItemClicked(const QString &text)
{
    m_textLabel->setText(text);
    emit itemClicked();
}

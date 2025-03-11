#include "CustomLayoutPanel.h"
#include "ui_CustomLayoutPanel.h"
#include "MyDebug.h"
#include "CustomLayoutData.h"
#include <QFontMetrics>
#include <QPainter>

CustomLayoutPanel::CustomLayoutPanel(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CustomLayoutPanel)
{
    ui->setupUi(this);

    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
}

CustomLayoutPanel::~CustomLayoutPanel()
{
    delete ui;
}

void CustomLayoutPanel::initializeData(int screen, const CustomLayoutKey &currentKey, const QStringList &names)
{
    QFontMetrics fm(ui->listWidget->font());

    ui->listWidget->clear();
    QListWidgetItem *currentItem = nullptr;
    for (int i = 0; i < names.size(); ++i) {
        auto name = names.at(i);
        const QString &elidedText = fm.elidedText(name, Qt::ElideRight, ui->listWidget->width() - 40);
        QListWidgetItem *item = new QListWidgetItem(elidedText);
        item->setData(RealCustomLayoutNameRole, name);
        item->setToolTip(name);
        ui->listWidget->addItem(item);
        if (currentKey.screen() == screen && currentKey.type() == CustomLayoutKey::CustomType && currentKey.name() == name) {
            currentItem = item;
        }
    }
    if (currentItem) {
        ui->listWidget->setCurrentItem(currentItem);
    }
}

void CustomLayoutPanel::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    //
    painter.setPen(QPen(QColor(191, 191, 191), 2));
    painter.setBrush(Qt::white);
    static const QPoint points[7] = {
        QPoint(m_marginLeft, m_marginTop),
        QPoint(m_marginLeft + width() / 2 - 8, m_marginTop),
        QPoint(m_marginLeft + width() / 2, m_marginTop - 10),
        QPoint(m_marginLeft + width() / 2 + 8, m_marginTop),
        QPoint(width() - m_marginRight, m_marginTop),
        QPoint(width() - m_marginRight, height() - m_marginBottom),
        QPoint(m_marginLeft, height() - m_marginBottom)
    };
    painter.drawPolygon(points, 7);
}

void CustomLayoutPanel::hideEvent(QHideEvent *)
{
    emit closed();
}

void CustomLayoutPanel::on_listWidget_itemClicked(QListWidgetItem *item)
{
    close();

    emit itemClicked(item->data(RealCustomLayoutNameRole).toString());
}

void CustomLayoutPanel::on_toolButtonSettings_clicked()
{
    ui->toolButtonSettings->clearUnderMouse();
    ui->toolButtonSettings->clearFocus();

    emit settingClicked();
}

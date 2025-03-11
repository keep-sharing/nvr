#include "presetitembutton.h"
#include "ui_presetitembutton.h"
#include "presetitemdelegate.h"
#include <QFile>
#include <QtDebug>

PresetItemButton::PresetItemButton(int row, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PresetItemButton)
    , m_currentRow(row)
{
    ui->setupUi(this);
}

PresetItemButton::~PresetItemButton()
{
    delete ui;
}

void PresetItemButton::setTheme(int theme)
{
    QString filePath;
    switch (theme) {
    case PresetItemDelegate::WhiteTheme:
        filePath = ":/style/style/presetitembutton_style_white.qss";
        break;
    case PresetItemDelegate::GrayTheme:
        filePath = ":/style/style/presetitembutton_style_gray.qss";
        break;
    }
    QFile file(filePath);
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QString strStyle = file.readAll();
        setStyleSheet(strStyle);
    } else {
        qWarning() << QString("Liveview load style failed.");
    }
    file.close();
}

void PresetItemButton::setButtonType(int type)
{
    switch (type) {
    case PresetItemDelegate::ItemDisable:
        ui->toolButton_save->setEnabled(true);
        ui->toolButton_delete->setEnabled(false);
        ui->toolButton_play->setEnabled(false);
        break;
    case PresetItemDelegate::ItemNormal:
        ui->toolButton_save->setEnabled(true);
        ui->toolButton_delete->setEnabled(true);
        ui->toolButton_play->setEnabled(true);
        break;
    case PresetItemDelegate::ItemValidDefault:
        ui->toolButton_save->setEnabled(false);
        ui->toolButton_delete->setEnabled(false);
        ui->toolButton_play->setEnabled(true);
        break;
    case PresetItemDelegate::ItemLimitsDisable:
      ui->toolButton_save->setEnabled(true);
      ui->toolButton_delete->setEnabled(false);
      ui->toolButton_play->setEnabled(false);
      break;
    case PresetItemDelegate::ItemLimits:
        ui->toolButton_save->setEnabled(true);
        ui->toolButton_delete->setEnabled(true);
        ui->toolButton_play->setEnabled(false);
        break;
    case PresetItemDelegate::ItemInvalidDefault:
        ui->toolButton_save->setEnabled(false);
        ui->toolButton_delete->setEnabled(false);
        ui->toolButton_play->setEnabled(false);
        break;
    default:
        break;
    }
}

void PresetItemButton::on_toolButton_save_clicked()
{
    ui->toolButton_save->setAttribute(Qt::WA_UnderMouse, false);

    emit buttonClicked(m_currentRow, 1);
}

void PresetItemButton::on_toolButton_delete_clicked()
{
    ui->toolButton_delete->setAttribute(Qt::WA_UnderMouse, false);

    emit buttonClicked(m_currentRow, 2);
}

void PresetItemButton::on_toolButton_play_clicked()
{
    ui->toolButton_play->setAttribute(Qt::WA_UnderMouse, false);

    emit buttonClicked(m_currentRow, 0);
}

#include "patternitembutton.h"
#include "ui_patternitembutton.h"
#include <QtDebug>
#include "patternitemdelegate.h"
#include "msuser.h"
#include "MessageBox.h"
#include "MsGlobal.h"

PatternItemButton::PatternItemButton(int row, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PatternItemButton),
    m_currentRow(row)
{
    ui->setupUi(this);
}

PatternItemButton::~PatternItemButton()
{
    delete ui;
}

void PatternItemButton::setButtonType(int type)
{
    switch (type)
    {
    case PatternItemDelegate::ItemDisable:
        ui->toolButton_play->setEnabled(false);
        ui->toolButton_record->setEnabled(true);
        ui->toolButton_record->setChecked(false);
        ui->toolButton_delete->setEnabled(false);
        break;
    case PatternItemDelegate::ItemEnable:
        setPlayButtonState("play");
        ui->toolButton_play->setEnabled(true);
        ui->toolButton_record->setEnabled(true);
        ui->toolButton_record->setChecked(false);
        ui->toolButton_delete->setEnabled(true);
        break;
    case PatternItemDelegate::ItemPlaying:
        setPlayButtonState("stop");
        ui->toolButton_play->setEnabled(true);
        ui->toolButton_record->setEnabled(true);
        ui->toolButton_record->setChecked(false);
        ui->toolButton_delete->setEnabled(true);
        break;
    case PatternItemDelegate::ItemRecording:
        ui->toolButton_play->setEnabled(false);
        ui->toolButton_record->setEnabled(true);
        ui->toolButton_record->setChecked(true);
        ui->toolButton_delete->setEnabled(false);
        break;
    default:
        break;
    }
}

void PatternItemButton::setPlayButtonState(const QString &state)
{
    ui->toolButton_play->setProperty("state", state);
    ui->toolButton_play->style()->unpolish(ui->toolButton_play);
    ui->toolButton_play->style()->polish(ui->toolButton_play);
}

void PatternItemButton::on_toolButton_play_clicked()
{
    if (!gMsUser.checkBasicPermission(PERM_MODE_LIVE, PERM_LIVE_PTZCONTROL))
    {
        ui->toolButton_play->setAttribute(Qt::WA_UnderMouse, false);
        ShowMessageBox(GET_TEXT("COMMON/1011","Insufficient User Permissions."));
        return;
    }

    const QString &state = ui->toolButton_play->property("state").toString();
    if (state == "play")
    {
        setPlayButtonState("stop");
    }
    else if (state == "stop")
    {
        setPlayButtonState("play");
    }
    emit buttonClicked(m_currentRow, 0);
}

void PatternItemButton::on_toolButton_record_clicked()
{
    if (!gMsUser.checkBasicPermission(PERM_MODE_LIVE, PERM_LIVE_PTZSETTINGS))
    {
        ui->toolButton_record->setAttribute(Qt::WA_UnderMouse, false);
        ShowMessageBox(GET_TEXT("COMMON/1011","Insufficient User Permissions."));
        return;
    }

    emit buttonClicked(m_currentRow, 1);
}

void PatternItemButton::on_toolButton_delete_clicked()
{
    if (!gMsUser.checkBasicPermission(PERM_MODE_LIVE, PERM_LIVE_PTZSETTINGS))
    {
        ui->toolButton_delete->setAttribute(Qt::WA_UnderMouse, false);
        ShowMessageBox(GET_TEXT("COMMON/1011","Insufficient User Permissions."));
        return;
    }

    emit buttonClicked(m_currentRow, 2);
}

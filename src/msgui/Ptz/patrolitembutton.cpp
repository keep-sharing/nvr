#include "patrolitembutton.h"
#include "ui_patrolitembutton.h"
#include "patrolitemdelegate.h"
#include <QtDebug>
#include "msuser.h"
#include "MessageBox.h"
#include "MsGlobal.h"

PatrolItemButton::PatrolItemButton(int row, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PatrolItemButton),
    m_currentRow(row)
{
    ui->setupUi(this);
}

PatrolItemButton::~PatrolItemButton()
{
    delete ui;
}

void PatrolItemButton::setButtonType(int type)
{
    switch (type)
    {
    case PatrolItemDelegate::ItemDisable:
        ui->toolButton_play->setEnabled(false);
        ui->toolButton_setting->setEnabled(true);
        ui->toolButton_delete->setEnabled(false);
        break;
    case PatrolItemDelegate::ItemEnable:
        setPlayButtonState("play");
        ui->toolButton_play->setEnabled(true);
        ui->toolButton_setting->setEnabled(true);
        ui->toolButton_delete->setEnabled(true);
        break;
    case PatrolItemDelegate::ItemPlaying:
        setPlayButtonState("stop");
        ui->toolButton_play->setEnabled(true);
        ui->toolButton_setting->setEnabled(true);
        ui->toolButton_delete->setEnabled(true);
        break;
    default:
        break;
    }
}

void PatrolItemButton::setPlayButtonState(const QString &state)
{
    ui->toolButton_play->setProperty("state", state);
    ui->toolButton_play->style()->unpolish(ui->toolButton_play);
    ui->toolButton_play->style()->polish(ui->toolButton_play);
}

void PatrolItemButton::on_toolButton_play_clicked()
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

void PatrolItemButton::on_toolButton_setting_clicked()
{
    if (!gMsUser.checkBasicPermission(PERM_MODE_LIVE, PERM_LIVE_PTZSETTINGS))
    {
        ui->toolButton_setting->setAttribute(Qt::WA_UnderMouse, false);
        ShowMessageBox(GET_TEXT("COMMON/1011","Insufficient User Permissions."));
        return;
    }

    emit buttonClicked(m_currentRow, 1);
}

void PatrolItemButton::on_toolButton_delete_clicked()
{
    if (!gMsUser.checkBasicPermission(PERM_MODE_LIVE, PERM_LIVE_PTZSETTINGS))
    {
        ui->toolButton_delete->setAttribute(Qt::WA_UnderMouse, false);
        ShowMessageBox(GET_TEXT("COMMON/1011","Insufficient User Permissions."));
        return;
    }

    emit buttonClicked(m_currentRow, 2);
}

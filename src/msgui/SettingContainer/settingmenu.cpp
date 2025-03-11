#include "settingmenu.h"
#include "ui_settingmenu.h"
#include "AutoLogout.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include <QtDebug>

SettingMenu::SettingMenu(QWidget *parent)
    : BaseShadowDialog(parent)
    , ui(new Ui::SettingMenu)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::WindowMinMaxButtonsHint);

    QList<QToolButton *> buttonList = ui->widget_background->findChildren<QToolButton *>();
    for (int i = 0; i < buttonList.size(); ++i) {
        QToolButton *button = buttonList.at(i);
        button->setMouseTracking(true);
        button->installEventFilter(this);
    }

    //
    if (qMsNvr->isSlaveMode()) {
        ui->toolButton_smart->hide();
        ui->toolButton_retrieve->hide();
        ui->toolButton_camera->hide();
    }

    connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
    connect(&gAutoLogout, SIGNAL(logouted()), this, SLOT(close()));
    onLanguageChanged();
}

SettingMenu::~SettingMenu()
{
    delete ui;
}

bool SettingMenu::eventFilter(QObject *obj, QEvent *e)
{
    switch (e->type()) {
    case QEvent::MouseMove: {
        QToolButton *button = qobject_cast<QToolButton *>(obj);
        if (button) {
            button->setFocus();
        }
        return true;
    }
    default:
        break;
    }
    return BaseShadowDialog::eventFilter(obj, e);
}

void SettingMenu::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)
}

void SettingMenu::onLanguageChanged()
{
    ui->toolButton_playback->setText(GET_TEXT("MENU/10002", "Playback"));
    ui->toolButton_retrieve->setText(GET_TEXT("MENU/10014", "Retrieve"));
    ui->toolButton_smart->setText(GET_TEXT("ANPR/103054", "Smart Analysis"));
    ui->toolButton_camera->setText(GET_TEXT("MENU/10003", "Camera"));
    ui->toolButton_storage->setText(GET_TEXT("RECORDADVANCE/91019", "Storage"));
    ui->toolButton_event->setText(GET_TEXT("MENU/10005", "Event"));
    ui->toolButton_setting->setText(GET_TEXT("MENU/10007", "Settings"));
    ui->toolButton_status->setText(GET_TEXT("MENU/10006", "Status"));
    ui->toolButton_logout->setText(GET_TEXT("MENU/10012", "Logout"));
}

void SettingMenu::on_toolButton_playback_clicked()
{
    close();
    emit settingMenu(MainMenu::ItemPlayback);
}

void SettingMenu::on_toolButton_retrieve_clicked()
{
    close();
    emit settingMenu(MainMenu::ItemRetrieve);
}

void SettingMenu::on_toolButton_smart_clicked()
{
    close();
    emit settingMenu(MainMenu::ItemSmart);
}

void SettingMenu::on_toolButton_camera_clicked()
{
    close();
    emit settingMenu(MainMenu::ItemCamera);
}

void SettingMenu::on_toolButton_storage_clicked()
{
    close();
    emit settingMenu(MainMenu::ItemStorage);
}

void SettingMenu::on_toolButton_event_clicked()
{
    close();
    emit settingMenu(MainMenu::ItemEvent);
}

void SettingMenu::on_toolButton_setting_clicked()
{
    close();
    emit settingMenu(MainMenu::ItemSettings);
}

void SettingMenu::on_toolButton_status_clicked()
{
    close();
    emit settingMenu(MainMenu::ItemStatus);
}

void SettingMenu::on_toolButton_logout_clicked()
{
    close();
    emit settingMenu(MainMenu::ItemLogout);
}

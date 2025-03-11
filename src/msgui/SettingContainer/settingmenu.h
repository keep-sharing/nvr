#ifndef SETTINGMENU_H
#define SETTINGMENU_H

#include "BaseShadowDialog.h"
#include "MainMenu.h"

namespace Ui {
class SettingMenu;
}

class SettingMenu : public BaseShadowDialog
{
    Q_OBJECT

public:
    explicit SettingMenu(QWidget *parent = nullptr);
    ~SettingMenu();

protected:
    bool eventFilter(QObject *obj, QEvent *e) override;
    void showEvent(QShowEvent *event) override;

signals:
    void settingMenu(const MainMenu::MenuItem &item);

private slots:
    void onLanguageChanged();

    void on_toolButton_playback_clicked();
    void on_toolButton_retrieve_clicked();
    void on_toolButton_smart_clicked();
    void on_toolButton_camera_clicked();
    void on_toolButton_storage_clicked();
    void on_toolButton_event_clicked();
    void on_toolButton_setting_clicked();
    void on_toolButton_status_clicked();
    void on_toolButton_logout_clicked();

private:
    Ui::SettingMenu *ui;
};

#endif // SETTINGMENU_H

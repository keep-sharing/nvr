#ifndef MAINMENU_H
#define MAINMENU_H

#include "BaseWidget.h"

class QPropertyAnimation;
class MyToolButton;

namespace Ui {
class MainMenu;
}

class MainMenu : public BaseWidget
{
    Q_OBJECT

public:
    enum MenuItem
    {
        ItemPlayback,
        ItemRetrieve,
        ItemSmart,
        ItemCamera,
        ItemStorage,
        ItemEvent,
        ItemSettings,
        ItemStatus,
        ItemLogout
    };

    explicit MainMenu(QWidget *parent = 0);
    ~MainMenu();

    static MainMenu *instance();

    void showLogout();

    void setTempShow();
    void showOrHide(const QPoint &point);
    bool isShow();

    void animateShow();
    void animateHide();

    void focusPreviousChild();
    void focusNextChild();

    QWidget *test_randomMenuButton();

protected:
    void returnPressed() override;

    bool isAddToVisibleList();

    NetworkResult deal_Dial_Insid_Add() override;
    NetworkResult deal_Dial_Insid_Sub() override;

private:
    bool isMouseEnter(const QPoint &mousePos);
    bool isMouseLeave(const QPoint &mousePos);

    void dealMenuClicked(const MainMenu::MenuItem &item);

signals:
    void itemClicked(const MainMenu::MenuItem &item);

private slots:
    void onLanguageChanged();

    void animationFinished();

    void onToolButtonMouseEnter();

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
    static MainMenu *self;
    Ui::MainMenu *ui;

    QList<MyToolButton *> m_items;

    int m_width = 250;
    QRect m_enterRect;
    QRect m_leaveRect;
    QPropertyAnimation *m_animation;
    bool m_show = false;

    bool m_tempShow = false;
};

#endif // MAINMENU_H

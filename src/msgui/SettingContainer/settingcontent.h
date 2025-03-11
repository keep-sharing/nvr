#ifndef SETTINGCONTENT_H
#define SETTINGCONTENT_H

#include "basesetting.h"
#include "MainMenu.h"
#include "BaseWidget.h"
#include "normallabel.h"
#include "settingmenu.h"

#include <QTimer>

class QListWidgetItem;

namespace Ui {
class SettingContent;
}

class SettingContent : public BaseWidget {
    Q_OBJECT

public:
    explicit SettingContent(QWidget *parent = 0);
    ~SettingContent();

    static SettingContent *instance();
    static SettingContent *s_settingContent;

    void dealMessage(MessageReceive *message);
    void goSetting(const MainMenu::MenuItem &item, int subItem = -1);

    void initializeSettingTimeout();
    void refreshTimeout();

    void dealMouseMove(const QPoint &pos);

    //
    void closeSetting();
    void closeToLiveView();

    //
    QPoint globalDownloadPos();
    QRect globalDownloadRect();

    //
    QRect globalContentRect() const;

    //
    void showCameraManagenment();
    void showDiskSetting();
    void showLayoutSetting();

    bool canAutoLogout();

    //
    NetworkResult dealNetworkCommond(const QString &commond) override;
    void networkTab1();
    void networkTab1_Prev();
    NetworkResult networkTab2();
    NetworkResult networkTab2_Prev();

    //
    QList<QWidget *> test_itemButtonList();

protected:
    bool eventFilter(QObject *, QEvent *) override;

    void showEvent(QShowEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

    void escapePressed() override;
    void returnPressed() override;

    bool isAddToVisibleList() override;

private:
    bool removeSetting();

    void updateTimeoutTipValue();
    void clearTimeoutTipValue();

signals:
    void closed();

private slots:
    void onSettingMenu(const MainMenu::MenuItem &item);
    void onLanguageChanged();

    void onSettingTimeout();

    void settingClosed();

    void on_toolButton_liveview_clicked();
    void onSettingItemClicked(const SettingItemInfo &info);

    void on_toolButton_menu_clicked();

    void onClickTimer();

private:
    Ui::SettingContent *ui;

    int m_settingTimeoutInterval = 0;
    int m_settingTimeoutCount = 0;
    QTimer *m_timerSettingTimeout;

    BaseSetting *m_setting = nullptr;
    SettingMenu *m_menu = nullptr;
    bool m_isAboutToClose = false;

    NormalLabel *m_labelNote = nullptr;

    //click counter
    QTimer *m_clickTimer = nullptr;
    int m_clickCount = 0;
};

#endif // SETTINGCONTENT_H

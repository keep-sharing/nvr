#ifndef CUSTOMLAYOUT_H
#define CUSTOMLAYOUT_H

#include "AbstractSettingPage.h"
#include "CustomLayoutInfoList.h"
#include <QMap>

extern "C" {
#include "msg.h"
}

class PageLayout;
class CustomLayoutPanel;
class CustomLayoutDialog;
class MyButtonGroup;
class QToolButton;

namespace Ui {
class LayoutSettings;
}

class LayoutSettings : public AbstractSettingPage {
    Q_OBJECT

public:
    explicit LayoutSettings(QWidget *parent = nullptr);
    ~LayoutSettings();

    static LayoutSettings *instance();

    int channelFromGlobalIndex(const CustomLayoutKey &key, int index);
    void setChannelFromGlobalIndex(const CustomLayoutKey &key, int index, int channel);

    virtual void initializeData() override;
    virtual void processMessage(MessageReceive *message) override;

private:
    //更新布局类型选中状态
    void updateLayoutModeButtonState();

    void showPreviewLayouts(const CustomLayoutKey &key);

private slots:
    void onLanguageChanged();

    void on_comboBoxScreen_indexSet(int index);

    void onChannelButtonClicked(int channel);

    void onDefaultLayoutClicked(int id);
    void onCustomLayoutClicked(const QString &name);
    void onLayoutModeClicked(const CustomLayoutKey &key);
    void onLayoutPageClicked(const CustomLayoutKey &key, int page);

    void on_toolButtonCustom_clicked();
    void onCustomLayoutSettingClicked();
    void onCustomLayoutPanelClosed();

    void onCustomLayoutSaved();
    void onCustomLayoutClosed();

    void on_pushButton_reset_clicked();
    void on_pushButton_apply_clicked();
    void on_pushButton_back_clicked();

private:
    static LayoutSettings *s_customLayout;

    Ui::LayoutSettings *ui;

    int m_currentScreen = SCREEN_MAIN;
    CustomLayoutKey m_currentKey;

    MyButtonGroup *m_channelButtonGroup = nullptr;

    MyButtonGroup *m_layoutButtonGroup = nullptr;
    QMap<QString, QToolButton *> m_modeButtons;

    QList<PageLayout *> m_pageList;
    PageLayout *m_currentPage = nullptr;

    CustomLayoutPanel *m_customPanel = nullptr;
    CustomLayoutDialog *m_customLayoutDialog = nullptr;
    CustomLayoutInfoList m_allLayouts;
};

#endif // CUSTOMLAYOUT_H

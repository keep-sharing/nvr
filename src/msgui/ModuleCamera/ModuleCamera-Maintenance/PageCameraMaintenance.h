#ifndef PAGECAMERAMAINTENANCE_H
#define PAGECAMERAMAINTENANCE_H

#include "abstractcamerapage.h"
#include <QMap>

class AbstractSettingTab;

namespace Ui {
class CameraMaintenance;
}

class PageCameraMaintenance : public AbstractCameraPage {
    Q_OBJECT

    enum Tab {
        TabNone,
        TabLocal,
        TabOnline,
        TabConfiguration,
        TabReboot,
        TabReset,
        TabDiagnosisInfomation,
        TabLog
    };

public:
    explicit PageCameraMaintenance(QWidget *parent = nullptr);
    ~PageCameraMaintenance();

    void initializeData() override;
    void dealMessage(MessageReceive *message) override;

    bool isCloseable() override;
    bool isChangeable() override;
    bool canAutoLogout() override;

protected:
    void onLanguageChanged();

private:
    AbstractSettingTab *currentPage();

private slots:
    void onTabClicked(int index);

private:
    Ui::CameraMaintenance *ui;

    int m_currentTab = 0;
    QMap<int, AbstractSettingTab *> m_tabMap;
};

#endif // PAGECAMERAMAINTENANCE_H

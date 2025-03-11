#ifndef PTZCONFIGURATION_H
#define PTZCONFIGURATION_H

#include "abstractcamerapage.h"
#include <QMap>

class PtzBasePage;

namespace Ui {
class PtzConfigurationManager;
}

class PagePtzConfiguration : public AbstractCameraPage {
    Q_OBJECT

    enum PageType {
        PageNone,
        PageBasic,
        PageAutoHome,
        PagePTZLimit,
        PageInitialPosition,
        PagePrivacyMask,
        PageScheduledTasks,
        PageAutoTracking,
        PageConfigClear,
        PageWiper,
        PageAdvanced
    };

public:
    explicit PagePtzConfiguration(QWidget *parent = nullptr);
    ~PagePtzConfiguration() override;

    void initializeData() override;
    void dealMessage(MessageReceive *message) override;

private slots:
    void onTabClicked(int index);

private:
    Ui::PtzConfigurationManager *ui;

    PageType m_currentPageType = PageNone;
    QMap<PageType, PtzBasePage *> m_pageMap;
};

#endif // PTZCONFIGURATION_H

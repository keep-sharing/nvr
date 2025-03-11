#ifndef PAGEFACEDETECTIONSETTINGS_H
#define PAGEFACEDETECTIONSETTINGS_H

#include "AbstractSettingPage.h"

class AbstractSettingTab;

namespace Ui {
class FaceDetectionSettings;
}

class PageFaceDetectionSettings : public AbstractSettingPage
{
    Q_OBJECT

    enum SETTINGS_TAB {
        TAB_NONE,
        TAB_FACE_CAPTURE,
        TAB_ADVANCED
    };

public:
    explicit PageFaceDetectionSettings(QWidget *parent = nullptr);
    ~PageFaceDetectionSettings();

    void initializeData() override;

private slots:
    void onTabClicked(int index);

private:
    Ui::FaceDetectionSettings *ui;
    SETTINGS_TAB m_currentPageTab = TAB_NONE;
    QMap<SETTINGS_TAB, AbstractSettingTab *> m_typeMap;
};

#endif // PAGEFACEDETECTIONSETTINGS_H

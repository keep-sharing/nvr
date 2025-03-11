#pragma once

#include "abstractcamerapage.h"

namespace Ui {
class AdvancedSettings;
}

class AdvancedSettingsPrivate;
class AbstractAdvancedSettingsPage;
class AdvancedSettings : public AbstractCameraPage
{
    Q_OBJECT

public:
    explicit AdvancedSettings(QWidget *parent = 0);
    ~AdvancedSettings();
    virtual void initializeData() override;
    virtual void dealMessage(MessageReceive *message) override;
    void setDrawWidget(QWidget *widget);

public slots:

private:
    Ui::AdvancedSettings *ui;
    AdvancedSettingsPrivate *d;
    friend AdvancedSettingsPrivate;
};


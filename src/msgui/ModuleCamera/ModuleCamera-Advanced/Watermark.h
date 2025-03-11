#pragma once

#include "AbstractAdvancedSettingsPage.h"

namespace Ui {
class Watermark;
}

class WatermarkPrivate;
class Watermark : public AbstractAdvancedSettingsPage {
    Q_OBJECT

public:
    explicit Watermark(QWidget *parent = 0);
    ~Watermark();

    void initializeData() override;
    void processMessage(MessageReceive *message) override;

private:
    bool isInputValid();

public slots:
    void on_pushButton_apply_clicked();
    void on_pushButton_back_clicked();
    void on_pushButton_copy_clicked();
    void on_comboBox_watermark_currentIndexChanged(int);

private:
    Ui::Watermark *ui;
    WatermarkPrivate *d;
    friend WatermarkPrivate;
};

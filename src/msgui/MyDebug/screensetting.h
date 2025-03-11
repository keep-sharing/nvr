#ifndef SCREENSETTING_H
#define SCREENSETTING_H

#include "BaseShadowDialog.h"
#include <QLabel>

namespace Ui {
class ScreenSetting;
}

class ScreenSetting : public BaseShadowDialog
{
    Q_OBJECT

public:
    explicit ScreenSetting(QWidget *parent = nullptr);
    ~ScreenSetting();

    void initializeData();

private slots:
    void on_pushButton_set_clicked();

    void on_pushButton_default_clicked();

    void on_pushButton_refresh_clicked();

    void on_toolButton_close_clicked();

private:
    Ui::ScreenSetting *ui;

    QLabel *m_label = nullptr;
};

#endif // SCREENSETTING_H

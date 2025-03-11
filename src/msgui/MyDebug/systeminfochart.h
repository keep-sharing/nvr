#ifndef SYSTEMINFOCHART_H
#define SYSTEMINFOCHART_H

#include "BaseShadowDialog.h"

namespace Ui {
class SystemInfoChart;
}

class SystemInfoChart : public BaseShadowDialog
{
    Q_OBJECT

public:
    explicit SystemInfoChart(QWidget *parent = nullptr);
    ~SystemInfoChart();

    void appendValue(int index, int value);

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void on_toolButton_close_clicked();

private:
    Ui::SystemInfoChart *ui;
};

#endif // SYSTEMINFOCHART_H

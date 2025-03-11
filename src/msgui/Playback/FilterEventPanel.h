#ifndef FILTEREVENTPANEL_H
#define FILTEREVENTPANEL_H

#include "BasePlayback.h"
#include "DateTimeRange.h"
#include "SmartSpeedDebug.h"

namespace Ui {
class FilterEventPanel;
}

class MyCheckBox;

class FilterEventPanel : public BasePlayback {
    Q_OBJECT

public:
    explicit FilterEventPanel(QWidget *parent = nullptr);
    ~FilterEventPanel();
    void initializeData();

signals:
    void onFilterEventChange();

protected:
    void mousePressEvent(QMouseEvent *) override;
  private:
    void closePanel();

public slots:
    void onFilterEventPanelButtonClicked(int x, int y);

private slots:
    void onLanguageChanged();
    void onPushButtonClicked(bool isEventOnly);
    void on_pushButtonOverrideSearch_clicked();
    void on_pushButtonEventOnlySearch_clicked();
    void on_checkBoxFilter_clicked();

private:
    Ui::FilterEventPanel *ui;
    int m_filterEvent = INFO_MAJOR_NONE;
};

#endif // FILTEREVENTPANEL_H

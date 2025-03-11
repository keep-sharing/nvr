#ifndef HOLIDAYSETTING_H
#define HOLIDAYSETTING_H

#include "AbstractSettingPage.h"
#include "itembuttonwidget.h"

#include <QWidget>

extern "C" {
#include "msdb.h"
}

class HolidayEdit;

namespace Ui {
class HolidaySetting;
}

class HolidaySetting : public AbstractSettingPage {
    Q_OBJECT

public:
    enum HolidayColumn {
        ColumnID,
        ColumnName,
        ColumnStatus,
        ColumnStartDate,
        ColumnEndData,
        ColumnEdit
    };

    explicit HolidaySetting(QWidget *parent = nullptr);
    ~HolidaySetting();

    void initializeData() override;

    void processMessage(MessageReceive *message) override;

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    QString weekDateString(int month, int week, int day) const;
    void refreshTable(int row);

private slots:
    void onLanguageChanged();

    void onSaveFinished();

    void onTableItemClicked(int row, int column);
    void on_pushButtonApply_clicked();
    void on_pushButton_back_clicked();

private:
    Ui::HolidaySetting *ui;

    holiday *m_dbHolidayList = nullptr;
    HolidayEdit *m_holidayEdit = nullptr;
};

#endif // HOLIDAYSETTING_H

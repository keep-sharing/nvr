#ifndef OCCUPANCYGROUPSETTINGS_H
#define OCCUPANCYGROUPSETTINGS_H

#include "BaseShadowDialog.h"

extern "C" {
#include "msdb.h"
}

class OccupancyGroupEdit;

namespace Ui {
class OccupancyGroupSettings;
}

class OccupancyGroupSettings : public BaseShadowDialog {
    Q_OBJECT

    enum GroupColumn {
        ColumnCheck,
        ColumnGroup,
        ColumnName,
        ColumnChannel,
        ColumnEdit,
        ColumnDelete
    };

public:
    explicit OccupancyGroupSettings(QWidget *parent = 0);
    ~OccupancyGroupSettings();

    void setSourceData(PEOPLECNT_SETTING *source_array);

private:
    void showGroupTable();
    QString channelString(const QString &text);

private slots:
    void onLanguageChanged();
    void onGroupTableClicked(int row, int column);

    void on_pushButtonOk_clicked();
    void on_pushButtonCancel_clicked();

private:
    Ui::OccupancyGroupSettings *ui;

    PEOPLECNT_SETTING *m_sourceData = nullptr;
    PEOPLECNT_SETTING *m_cacheData = nullptr;
    int m_rowCount = 0;

    OccupancyGroupEdit *m_groupEdit = nullptr;
};

#endif // OCCUPANCYGROUPSETTINGS_H

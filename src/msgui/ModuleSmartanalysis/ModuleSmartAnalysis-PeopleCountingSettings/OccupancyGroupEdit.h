#ifndef OCCUPANCYGROUPEDIT_H
#define OCCUPANCYGROUPEDIT_H

#include "BaseShadowDialog.h"

extern "C"
{
#include "msdb.h"
}

namespace Ui {
class OccupancyGroupEdit;
}

class OccupancyGroupEdit : public BaseShadowDialog
{
    Q_OBJECT

    enum MODE {
        MODE_NONE,
        MODE_ADD,
        MODE_EDIT
    };

public:
    explicit OccupancyGroupEdit(QWidget *parent = 0);
    ~OccupancyGroupEdit();

    int execAdd(PEOPLECNT_SETTING *settings, int index);
    int execEdit(PEOPLECNT_SETTING *settings, int index);

private slots:
    void onLanguageChanged();

    void on_pushButtonOk_clicked();
    void on_pushButtonCancel_clicked();

private:
    Ui::OccupancyGroupEdit *ui;

    PEOPLECNT_SETTING *m_settings = nullptr;
    MODE m_mode = MODE_NONE;
};

#endif // OCCUPANCYGROUPEDIT_H

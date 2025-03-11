#ifndef LOGDETAIL_H
#define LOGDETAIL_H

#include "BaseShadowDialog.h"
#include <QDateTime>

struct log_data;

namespace Ui {
class LogDetail;
}

class LogDetail : public BaseShadowDialog {
    Q_OBJECT

public:
    explicit LogDetail(QWidget *parent = nullptr);
    ~LogDetail();

    void setDetail(const QString &str);
    void showLog(const log_data &log);

    static bool isDiskType(int mainType, int subType);

signals:
    void previousLog();
    void nextLog();

private:
private slots:
    void onLanguageChanged();

    void on_pushButton_previous_clicked();
    void on_pushButton_next_clicked();
    void on_pushButton_back_clicked();

private:
    Ui::LogDetail *ui;
};

#endif // LOGDETAIL_H

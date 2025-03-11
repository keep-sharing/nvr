#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include "BaseShadowDialog.h"

namespace Ui {
class ProgressDialog;
}

class ProgressDialog : public BaseShadowDialog
{
    Q_OBJECT

public:
    explicit ProgressDialog(QWidget *parent = 0);
    ~ProgressDialog();

    static ProgressDialog *instance();

    void setTitle(const QString &text);
    void setMessage(const QString &text);

    void setCancelEnable(bool enable);

    void showProgress();
    void hideProgress();
    void setProgressValue(int value);

signals:
    void sig_cancel();

private slots:
    void onLanguageChanged();

    void on_pushButton_cancel_clicked();

private:
    static ProgressDialog *s_progressDialog;

    Ui::ProgressDialog *ui;
};

class ProgressContainer {
    inline explicit ProgressContainer(ProgressDialog *p)
    {
        m_progress = p;
        if (m_progress) {
            m_progress->showProgress();
        }
    }
    inline ~ProgressContainer()
    {
        if (m_progress) {
            m_progress->hideProgress();
        }
    }

private:
    ProgressDialog *m_progress = nullptr;
};

#endif // PROGRESSDIALOG_H

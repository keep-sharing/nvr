#ifndef PAGENETWORKTEST_H
#define PAGENETWORKTEST_H
#include <QWidget>
#include <QThread>
#include <QEventLoop>
#include <MyCmdProcess.h>

extern "C"
{

}

namespace Ui {
class TabNetworkTest;
}

class TabNetworkTest : public QWidget
{
    Q_OBJECT

public:
    explicit TabNetworkTest(QWidget *parent = 0);
    ~TabNetworkTest();

    void initializeData();
    void startPing();
    void stopPing();

protected:
    void hideEvent(QHideEvent *) override;

private:
    void showWait();
    void closeWait();
    void updateEnableState(bool enabled);
    QString DestinationAddress();

signals:
    void sendMessage(QString message);
    void finishPing();
    void sig_back();

private slots:
    void onLanguageChanged();
    void updataResult(QString cmdDate);
    void updataText(QString cmdDate);

    void on_textResult_textChanged();

    void on_pushButtonStart_clicked();
    void on_pushButtonStop_clicked();
    void on_pushButtonBack_clicked();

private:
    Ui::TabNetworkTest *ui;

    QThread *m_threadtest;
    MyCmdProcess *m_cmdp;

    bool m_pressStart = false;
};

#endif // PAGENETWORKTEST_H

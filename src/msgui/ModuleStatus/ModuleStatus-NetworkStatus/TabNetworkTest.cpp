#include "TabNetworkTest.h"
#include "ui_TabNetworkTest.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MsWaitting.h"
#include <QHostAddress>
#include <QProcess>
#include <QDebug>
TabNetworkTest::TabNetworkTest(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TabNetworkTest)
{
    ui->setupUi(this);
    ui->textResult->setReadOnly(true);
    QPalette pl = ui->textResult->palette();
    pl.setColor(QPalette::Base, QColor(190, 190, 190));
    ui->textResult->setPalette(pl);

    m_threadtest = new QThread(this);
    m_cmdp = new MyCmdProcess();
    m_cmdp->moveToThread(m_threadtest);

    connect(this, SIGNAL(sendMessage(QString)), m_cmdp, SLOT(resultValue(QString)));
    connect(m_cmdp, SIGNAL(cmdResult(QString )), this, SLOT(updataResult(QString)));
    connect(m_threadtest, SIGNAL(finished()), m_cmdp, SLOT(deleteLater()) );
    connect(m_cmdp, SIGNAL(cmdText(QString)), this, SLOT(updataText(QString)));
    connect(this, SIGNAL(finishPing()), m_cmdp, SLOT(cmdStop()));

    ui->lineEditDestination->setCheckMode(MyLineEdit::EmptyCheck);
    m_threadtest->start();

    onLanguageChanged();
}

TabNetworkTest::~TabNetworkTest()
{
    delete ui;
    m_threadtest->quit();
    m_threadtest->wait();
}

void TabNetworkTest::initializeData()
{
    if (!ui->pushButtonStart->isEnabled()) {
        stopPing();
    }
    m_pressStart = false;
    updateEnableState(true);
    ui->lineEditDestination->clear();
    ui->textResult->clear();
}

void TabNetworkTest::startPing()
{
    if (!ui->lineEditDestination->checkValid()) {
        updateEnableState(true);
        return;
    }
    m_pressStart = true;
    ui->textResult->clear();
    QString testIP = ui->lineEditDestination->text();
    ui->lineEditDestination->setEnabled(false);

    QString cmdDate = "ping -c 60 " + testIP;
    emit sendMessage(cmdDate);
}

void TabNetworkTest::updataResult(QString cmdDate)
{
    qDebug()<<"updata network test result"<<cmdDate;
    if (m_pressStart) {
        QRegExp rx("/(([1-9]\\d*\\.\\d*|0\\.\\d*[1-9]\\d*)|([0-9]*))(?=/)");//匹配一串大于等于0且后面跟着%的浮点数
        if (rx.indexIn(cmdDate) == -1) {
            rx.setPattern("packets transmitted, 0 packets received, 100% packet loss");
            if (rx.indexIn(cmdDate) != -1) {
                ui->textResult->setText(" " + GET_TEXT("PING/116008", "Request time out. "));
            } else {
                ui->textResult->setText( " " + GET_TEXT("PING/116007", "Network is unreachable."));
            }
        } else {
            //结果每行缩进一格
            int point = cmdDate.indexOf("---");
            cmdDate.insert(point, " ");
            point = cmdDate.lastIndexOf("---");
            cmdDate.insert(point + 4 , " ");
            point = cmdDate.indexOf("round");
            cmdDate.insert(point, " ");
            cmdDate.insert(0, " ");
            ui->textResult->setText(ui->textResult->toPlainText() + cmdDate);
        }
        m_pressStart = false;
        updateEnableState(true);
    }
}

void TabNetworkTest::updataText(QString cmdDate)
{
    QString strText ;
    if (ui->textResult->toPlainText().isEmpty()) {
        int start = cmdDate.indexOf("bytes");
        strText = " " + cmdDate.insert(start + 6, " ");
    } else {
        strText = " " + cmdDate;
    }
    ui->textResult->setText(ui->textResult->toPlainText() + strText);
}

void TabNetworkTest::stopPing()
{
    emit finishPing();
}

void TabNetworkTest::hideEvent(QHideEvent * event)
{
    QWidget::hideEvent(event);
    if (!ui->pushButtonStart->isEnabled()) {
        stopPing();
    }
}

void TabNetworkTest::showWait()
{
    //MsWaitting::showGlobalWait();
}

void TabNetworkTest::closeWait()
{
    //MsWaitting::closeGlobalWait();
}

void TabNetworkTest::updateEnableState(bool enabled)
{
    ui->lineEditDestination->setEnabled(enabled);
    ui->pushButtonStart->setEnabled(enabled);
    ui->pushButtonStop->setEnabled(!enabled);
    ui->pushButtonBack->clearFocus();
}

QString TabNetworkTest::DestinationAddress()
{
    return ui->lineEditDestination->text();
}

void TabNetworkTest::onLanguageChanged()
{
    ui->label_destination->setText(GET_TEXT("PING/116005", "Destination Address"));
    ui->label_testResult->setText(GET_TEXT("PING/116004", "Test Result"));
    ui->pushButtonBack->setText(GET_TEXT("COMMON/1002", "Back"));
    ui->pushButtonStart->setText(GET_TEXT("PING/116009", "Start"));
    ui->pushButtonStop->setText(GET_TEXT("PTZCONFIG/36038", "Stop"));
}

void TabNetworkTest::on_textResult_textChanged()
{
    ui->textResult->moveCursor(QTextCursor::End);
}

void TabNetworkTest::on_pushButtonStart_clicked()
{
    updateEnableState(false);
    startPing();
}

void TabNetworkTest::on_pushButtonStop_clicked()
{
    stopPing();
    updateEnableState(true);
}

void TabNetworkTest::on_pushButtonBack_clicked()
{
    emit sig_back();
}

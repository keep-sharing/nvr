#include "autotest.h"
#include "ui_autotest.h"

#include "BottomBar.h"
#include "LiveView.h"
#include "MessageBox.h"
#include "MsGlobal.h"
#include "MsWaitting.h"
#include "PlaybackWindow.h"
#include "settingcontent.h"

#include <QTest>

AutoTest *AutoTest::self = nullptr;
AutoTest::AutoTest(QWidget *parent)
    : BaseShadowDialog(parent)
    , ui(new Ui::AutoTest)
{
    ui->setupUi(this);

    QFile file(":/style/style/settingstyle.qss");
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        setStyleSheet(file.readAll());
    } else {
        qWarning() << QString("AutoTest load style failed.");
    }
    file.close();

    self = this;
    setWindowModality(Qt::NonModal);
    setTitleWidget(ui->label_title);
}

AutoTest::~AutoTest()
{
    self = nullptr;
    delete ui;
}

AutoTest *AutoTest::instance()
{
    return self;
}

void AutoTest::stopAllTest()
{
    if (m_timerLayoutChange) {
        m_timerLayoutChange->stop();
    }
    ui->pushButton_layoutChange->setText("已停止");
    ui->pushButton_layoutChange->setEnabled(true);
    if (m_timerMenuChange) {
        m_timerMenuChange->stop();
    }
    ui->pushButton_menuChange->setText("已停止");
    ui->pushButton_menuChange->setEnabled(true);
}

void AutoTest::onTimerLayoutChange()
{
    if (MsWaitting::hasWaitting()) {
        return;
    }
    if (LiveView::instance() && LiveView::instance()->isVisible()) {
        QWidget *layoutButton = BottomBar::instance()->liveviewBottomBar()->test_randomLayoutButton();
        QTest::mouseClick(layoutButton, Qt::LeftButton);

        m_testLayoutChangeCount++;
        ui->label_layoutChange_count->setText(QString("执行次数：%1").arg(m_testLayoutChangeCount));
    }
}

void AutoTest::onTimerMenuChange()
{
    //qWarning() << "AutoTest::onTimerMenuChange";
    if (MsWaitting::hasWaitting()) {
        return;
    }
    if (g_messageBox && g_messageBox->isVisible()) {
        g_messageBox->close();
        return;
    }
    if (LiveView::instance() && LiveView::instance()->isVisible()) {
        QWidget *menuButton = MainMenu::instance()->test_randomMenuButton();
        if (menuButton) {
            QTest::mouseClick(menuButton, Qt::LeftButton);
        }
        m_testMenuChangeCount++;
    } else if (PlaybackWindow::instance() && PlaybackWindow::instance()->isVisible()) {
        QTest::mouseClick(PlaybackWindow::instance(), Qt::RightButton);
    } else if (SettingContent::instance() && SettingContent::instance()->isVisible()) {
        static bool isReadyQuit = false;
        if (isReadyQuit) {
            isReadyQuit = false;
            QTest::mouseClick(SettingContent::instance(), Qt::RightButton);
        } else {
            if (m_itemButtonList.isEmpty()) {
                m_itemButtonList = SettingContent::instance()->test_itemButtonList();
                isReadyQuit = false;
            }
            if (m_itemButtonList.isEmpty()) {
                isReadyQuit = true;
            } else {
                QWidget *itemButton = m_itemButtonList.takeFirst();
                if (m_itemButtonList.isEmpty()) {
                    isReadyQuit = true;
                }
                QTest::mouseClick(itemButton, Qt::LeftButton);
            }
        }
    }
    ui->label_menuChangeCount->setText(QString("执行次数：%1").arg(m_testMenuChangeCount));
}

void AutoTest::on_pushButton_layoutChange_clicked()
{
    QString text = ui->pushButton_layoutChange->text();
    if (text.contains("停止")) {
        if (!LiveView::instance()->isVisible()) {
            ShowMessageBox("请在Liveview界面开启");
            return;
        }

        if (!m_timerLayoutChange) {
            m_timerLayoutChange = new QTimer(this);
            connect(m_timerLayoutChange, SIGNAL(timeout()), this, SLOT(onTimerLayoutChange()));
        }
        m_testLayoutChangeCount = 0;
        m_timerLayoutChange->start(3000);
        ui->pushButton_layoutChange->setText("已启动");
        ui->pushButton_menuChange->setEnabled(false);
    } else if (text.contains("启动")) {
        if (m_timerLayoutChange) {
            m_timerLayoutChange->stop();
        }
        ui->pushButton_layoutChange->setText("已停止");
        ui->pushButton_menuChange->setEnabled(true);
    }
}

void AutoTest::on_pushButton_menuChange_clicked()
{
    QString text = ui->pushButton_menuChange->text();
    if (text.contains("停止")) {
        if (!LiveView::instance()->isVisible()) {
            ShowMessageBox("请在Liveview界面开启");
            return;
        }

        if (!m_timerMenuChange) {
            m_timerMenuChange = new QTimer(this);
            connect(m_timerMenuChange, SIGNAL(timeout()), this, SLOT(onTimerMenuChange()));
        }
        m_testMenuChangeCount = 0;
        m_timerMenuChange->start(2000);
        ui->pushButton_menuChange->setText("已启动");
        ui->pushButton_layoutChange->setEnabled(false);
    } else if (text.contains("启动")) {
        if (m_timerMenuChange) {
            m_timerMenuChange->stop();
        }
        ui->pushButton_menuChange->setText("已停止");
        ui->pushButton_layoutChange->setEnabled(true);
    }
}

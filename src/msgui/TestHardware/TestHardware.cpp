#include "TestHardware.h"
#include "ui_TestHardware.h"
#include "MessageBox.h"
#include "MessageFilter.h"
#include "MsApplication.h"
#include "MsDevice.h"
#include "MsMessage.h"
#include "MsWaitting.h"
#include "MyDebug.h"
#include "SubControl.h"
#include "TestItemCommon.h"
#include "mainwindow.h"
#include "screencontroller.h"
#include <QProcess>

TestHardware *TestHardware::s_hardWare = nullptr;

TestHardware::TestHardware(QWidget *parent)
    : BaseShadowDialog(parent)
    , ui(new Ui::TestHardware)
{
    s_hardWare = this;

    ui->setupUi(this);
    setTitleWidget(ui->label_title);

    ui->comboBoxVideoCodec->clear();
    ui->comboBoxVideoCodec->addItem("H.264", VIDEO_FORMAT_H264);
    ui->comboBoxVideoCodec->addItem("H.265", VIDEO_FORMAT_H265);

    TestHardwareData *testData = new TestHardwareData(nullptr);
    connect(testData, SIGNAL(wizardMessage(QString)), this, SLOT(onShowWizardMessage(QString)));
    connect(testData, SIGNAL(message(QColor, QString)), this, SLOT(onShowMessage(QColor, QString)));

    m_testWizard = new TestWizard(this);
    connect(m_testWizard, SIGNAL(testNext()), this, SLOT(onWizardNext()));
    connect(m_testWizard, SIGNAL(testCancel()), this, SLOT(onWizardCancel()));

    //VIDEO 默认参数
    ui->lineEditIP->setText("192.168.5.190");
    ui->lineEditUser->setText("admin");
    ui->lineEditPassword->setText("1");
    ui->comboBoxVideoCodec->setCurrentIndex(ui->comboBoxVideoCodec->findText("H.264"));

    gMessageFilter.installMessageFilter(RESPONSE_FLAG_POWER_SHORT, this);
    gMessageFilter.installMessageFilter(RESPONSE_FLAG_POWER_LONG, this);
    gMessageFilter.installMessageFilter(RESPONSE_FLAG_RESOLUTION_CHANGED, this);

    connect(SubControl::instance(), SIGNAL(screenSwitched()), this, SLOT(onScreenSwitched()));

    pDisplayDb = new display;

    resize(1280, 900);
    initialize();
}

TestHardware::~TestHardware()
{
    delete pDisplayDb;
    pDisplayDb = nullptr;
    TestHardwareData::instance()->stopThread();
    TestHardwareData::instance()->deleteLater();
    s_hardWare = nullptr;
    delete ui;
}

TestHardware *TestHardware::instance()
{
    return s_hardWare;
}

void TestHardware::initialize()
{
    TestList list;
    ms_hw_test_get_list(&list);
    for (int i = list.count - 1; i > -1; --i) {
        QString name(list.entrys[i]);
        AbstractTestItem *item = new TestItemCommon(this);
        item->setModuleName(name);
        connect(item, SIGNAL(itemStarted(AbstractTestItem *)), this, SLOT(onItemStarted(AbstractTestItem *)));
        connect(item, SIGNAL(checkChanged(bool)), this, SLOT(onItemChecked(bool)));
        item->setChecked(true);
        ui->verticalLayoutItems->addWidget(item);

        m_testItems.append(item);
    }

    //mac
    char mac[32] = { 0 };
    read_mac_conf(mac);
    QString strMac;
    QString tempMac(mac);
    if (tempMac.isEmpty()) {
        strMac = qMsNvr->macList().first();
    } else {
        for (int i = 0; i < tempMac.size(); i += 2) {
            strMac.append(tempMac.mid(i, 2));
            if (i < tempMac.size() - 2) {
                strMac.append(":");
            }
        }
    }
    ui->lineEditMAC->setText(strMac);

    //hadrware
    ui->lineEditHardware->setText(qMsNvr->hardwareVersion());

    //software
    ui->lineEditSoftware->setText(qMsNvr->softwareVersion());

    //sn
    sendMessage(REQUEST_FLAG_GET_SYSINFO, nullptr, 0);

    //hdmi
    const struct device_info &sys_info = qMsNvr->deviceInfo();

    ui->labelOutput->hide();
    ui->comboBoxOutput->hide();
    ui->comboBoxOutput->clear();
    ui->comboBoxOutput->addItem("独立", 0);
    ui->comboBoxOutput->addItem("同步", 1);

    ui->comboBoxHDMI2->clear();
    ui->comboBoxHDMI2->addItem("不启用", 0);
    ui->comboBoxHDMI2->addItem("启用", 1);

    //
    QMap<int, QString> resolutionMap;
    resolutionMap.insert(OUTPUT_RESOLUTION_1080P, "1920 x 1080 / 60Hz");
    resolutionMap.insert(OUTPUT_RESOLUTION_720P, "1280 x 720 / 60Hz");
    resolutionMap.insert(OUTPUT_RESOLUTION_SXGA, "1280 x 1024");
    resolutionMap.insert(OUTPUT_RESOLUTION_XGA, "1024 x 768");
    resolutionMap.insert(OUTPUT_RESOLUTION_1080P50, "1920 x 1080 / 50Hz");
    resolutionMap.insert(OUTPUT_RESOLUTION_2160P30, "3840 x 2160 / 30Hz");
    resolutionMap.insert(OUTPUT_RESOLUTION_2160P60, "3840 x 2160 / 60Hz");
    ui->comboBoxScreen1->clear();
    ui->comboBoxScreen2->clear();
    for (auto iter = resolutionMap.constBegin(); iter != resolutionMap.constEnd(); ++iter) {
        int value = iter.key();
        QString text = iter.value();
        ui->comboBoxScreen1->addItem(text, value);
        ui->comboBoxScreen2->addItem(text, value);
    }
    //
    ui->comboBoxScreen2->removeItemFromData(OUTPUT_RESOLUTION_2160P60);
    ui->comboBoxScreen2->removeItemFromData(OUTPUT_RESOLUTION_2160P30);

    //
    QString strModel(sys_info.model);
    QString strPrefix(sys_info.prefix);
    if (qMsNvr->multiScreenSupport() == 2) {
        ui->labelOutput->show();
        ui->comboBoxOutput->show();
        ui->labelHDMI2->hide();
        ui->comboBoxHDMI2->hide();

        //这个功能仅3536平台，一个HDMI+一个VGA的型号支持（MS-N7016-UH，MS-N7032-UH，MS-N7016-UPH，MS-N7032-UPH）
        m_homologous = get_param_int(SQLITE_FILE_NAME, PARAM_HOMOLOGOUS, 1);
        ui->comboBoxOutput->setCurrentIndexFromData(m_homologous);
        switch (m_homologous) {
        case 0:
            //如果显示Independent，为HDMI和VGA异源输出，HDMI为主，VGA为辅
            ui->labelScreen1->setText("HDMI分辨率");
            ui->labelScreen2->setText("VGA分辨率");
            break;
        case 1:
            //如果选择Synchronous，为HDMI和VGA同源输出
            ui->labelScreen1->setText("HDMI/VGA输出");
            ui->labelScreen2->hide();
            ui->comboBoxScreen2->hide();
            break;
        }
    } else if (qMsNvr->multiScreenSupport() == 1) {
        ui->labelScreen1->setText("HDMI1/VGA1分辨率");
        ui->labelScreen2->setText("HDMI2/VGA2分辨率");
    } else {
        ui->labelScreen1->setText("HDMI/VGA输出");
        ui->labelHDMI2->hide();
        ui->comboBoxHDMI2->hide();
        ui->labelScreen2->hide();
        ui->comboBoxScreen2->hide();

        //
        if (strModel == QString("MS-N5016-UPT") || strModel == QString("MS-N5016-UT") || strModel == QString("MS-N5008-UPT") || strModel == QString("MS-N5008-UT")) {
            ui->comboBoxScreen1->removeItemFromData(OUTPUT_RESOLUTION_2160P60);
        }
        if (strPrefix == QString("1")) {
            ui->comboBoxScreen1->removeItemFromData(OUTPUT_RESOLUTION_2160P60);
        }
        if (qMsNvr->is3536c()) {
            ui->comboBoxScreen1->removeItemFromData(OUTPUT_RESOLUTION_2160P60);
        }
#ifdef _NT98323_
        ui->comboBoxScreen1->removeItemFromData(OUTPUT_RESOLUTION_2160P60);
#endif
    }
    updateResolutionInfo();

    //
    on_pushButtonMonitorUpdate_clicked();
}

void TestHardware::filterMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_POWER_SHORT:
        onShowMessage(Qt::darkGreen, "RESPONSE_FLAG_POWER_SHORT");
        break;
    case RESPONSE_FLAG_POWER_LONG:
        onShowMessage(Qt::darkGreen, "RESPONSE_FLAG_POWER_LONG");
        break;
    case RESPONSE_FLAG_RESOLUTION_CHANGED:
        updateResolutionInfo();
        break;
    }
}

void TestHardware::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_SYSINFO:
        ON_RESPONSE_FLAG_GET_SYSINFO(message);
        break;
    }
}

void TestHardware::setMainSubResolution(int main_resolution, int sub_resolution, int spot_resolution)
{
    Q_UNUSED(spot_resolution)

    REQ_SCREEN_S screen;
    screen.enScreen = SCREEN_MAIN;
    screen.enRes = (DisplayDcMode_e)main_resolution;
    sendMessageOnly(REQUEST_FLAG_SET_SCREEN, (void *)&screen, sizeof(screen));

    screen.enScreen = SCREEN_SUB;
    screen.enRes = (DisplayDcMode_e)sub_resolution;
    sendMessageOnly(REQUEST_FLAG_SET_SCREEN, (void *)&screen, sizeof(screen));

    MainWindow::instance()->hide();
    ScreenController::instance()->blackScreen(3000);
    ScreenController::instance()->setRefreshRate(DisplayDcMode_e(SubControl::instance()->currentScreen() == SCREEN_MAIN ? main_resolution : sub_resolution));
}

void TestHardware::readConfig()
{
    memset(pDisplayDb, 0, sizeof(display));
    qMsNvr->readDisplayInfo();
    *pDisplayDb = qMsNvr->displayInfo();

    ui->comboBoxScreen1->setCurrentIndexFromData(pDisplayDb->main_resolution);
    ui->comboBoxScreen2->setCurrentIndexFromData(pDisplayDb->sub_resolution);

    ui->comboBoxHDMI2->setCurrentIndex(pDisplayDb->sub_enable);
    ui->comboBoxScreen2->setEnabled(pDisplayDb->sub_enable);
}

QString TestHardware::updateMonitorInfo(QString message)
{
    QProcess m_process;
    QString resultStr = "";
    m_process.start(message);
    m_process.waitForFinished();
    bool support4K30P = false;
    bool support4K60P = false;
    QString text = m_process.readAllStandardOutput();
#if defined(_HI3536A_)
    QString basicStr = "basic cap";
    QString FormatStr = "format cap";
    QString hdmi14rx = R"(.*HDMI1.4 support(\s*:\s*)(\w+).*)";
    QString hdmi20rx = R"(.*HDMI2.0 support(\s*:\s*)(\w+).*)";
#else
    QString basicStr = "BasicCap";
    QString FormatStr = "FormatCap";
    QString hdmi14rx = R"(.*HDMI1.4Support(\s*:\s*)(\w+).*)";
    QString hdmi20rx = R"(.*HDMI2.0Support(\s*:\s*)(\w+).*)";
#endif
    if (!text.contains(basicStr)) {
        return QString("获取不到显示器信息"); //
    } else {
        QRegExp rx(hdmi14rx);
        if (rx.exactMatch(text)) {
            if (rx.cap(2).compare("YES") == 0) {
                support4K30P = true;
            }
            resultStr += QString("HDMI1.4 支持：" + rx.cap(2) + "\n");
        }
        QRegExp rx2(hdmi20rx);
        if (rx2.exactMatch(text)) {
            if (rx.cap(2).compare("YES") == 0) {
                support4K60P = true;
            }
            resultStr += QString("HDMI2.0 支持：" + rx2.cap(2) + "\n");
        }
        resultStr += QString("不支持以下分辨率：\n");
        if (!text.contains(FormatStr)) {
            if (!support4K30P) {
                resultStr += QString("3840 X 2160 / 30HZ \n");
            }
            if (!support4K60P) {
                resultStr += QString("3840 X 2160 / 60HZ");
            }
            return resultStr;
        } else {
            if (!text.contains("3840X2160P30")) {
                resultStr += QString("3840 X 2160 / 30HZ \n");
            }
            if (!text.contains("3840X2160P60")) {
                resultStr += QString("3840 X 2160 / 60HZ ");
            }
        }
    }
    return resultStr;
}

void TestHardware::ON_RESPONSE_FLAG_GET_SYSINFO(MessageReceive *message)
{
    if (!message->data) {
        return;
    }
    device_info *dbInfo = static_cast<device_info *>(message->data);
    ui->lineEditSN->setText(dbInfo->sncode);
}

void TestHardware::onShowWizardMessage(const QString &text)
{
    m_testWizard->showMessage(text);
}

void TestHardware::onShowMessage(QColor color, const QString &text)
{
    ui->plainTextEdit->appendMessage(color, text);
}

void TestHardware::updateResolutionInfo()
{
    readConfig();
    if (SubControl::instance()->isSubControl()) {
        ui->comboBoxScreen1->setEnabled(false);
        ui->comboBoxHDMI2->setEnabled(false);
        ui->comboBoxScreen2->setEnabled(true);
    } else {
        ui->comboBoxScreen1->setEnabled(true);
        ui->comboBoxHDMI2->setEnabled(true);
        ui->comboBoxScreen2->setEnabled(false);
    }
}

void TestHardware::onScreenSwitched()
{
    if (SubControl::instance()->isSubControl()) {
        ui->comboBoxHDMI2->setEnabled(false);
        ui->comboBoxScreen1->setEnabled(false);
        ui->comboBoxScreen2->setEnabled(true);
    } else {
        ui->comboBoxHDMI2->setEnabled(true);
        ui->comboBoxScreen1->setEnabled(true);
        ui->comboBoxScreen2->setEnabled(false);
    }
}

void TestHardware::onItemStarted(AbstractTestItem *item)
{
    onShowMessage(Qt::black, QString("=========== TEST %1 ===========").arg(item->moduleName()));
    for (int i = 0; i < m_testItems.size(); ++i) {
        AbstractTestItem *testItem = m_testItems.at(i);
        if (testItem == item) {
            m_currentTestIndex = i;
            testItem->setSelected(true);
        } else {
            testItem->setSelected(false);
        }
    }

    if (item->moduleName() == QString(TEST_NAME_VIDEO)) {
        TestVideo video;
        memset(&video, 0, sizeof(video));
        snprintf(video.ip, sizeof(video.ip), "%s", ui->lineEditIP->text().toLocal8Bit().constData());
        snprintf(video.user, sizeof(video.user), "%s", ui->lineEditUser->text().toLocal8Bit().constData());
        snprintf(video.password, sizeof(video.password), "%s", ui->lineEditPassword->text().toLocal8Bit().constData());
        video.format = static_cast<VideoFormat>(ui->comboBoxVideoCodec->itemData(ui->comboBoxVideoCodec->currentIndex()).toInt());

        QString text;
        text += "=========== Settings ===========";
        text += "\nIP:" + ui->lineEditIP->text();
        text += "\nUser:" + ui->lineEditUser->text();
        text += "\nPassword:" + ui->lineEditPassword->text();
        text += "\nVideoCodec:" + ui->comboBoxVideoCodec->currentText();
        onShowMessage(Qt::black, text);
        TestHardwareData::instance()->setVideoParams(video);
    }

    QPoint p = mapToGlobal(QPoint(0, 0));
    m_testWizard->startTest(item);
    if (!m_testWizard->isVisible()) {
        m_testWizard->show();
        m_testWizard->move(p.x() + width() - m_testWizard->width(), p.y() + height() / 2 - m_testWizard->height() / 2);
        //
        ui->groupBoxTest->setEnabled(false);
        ui->pushButtonStart->setEnabled(false);
        ui->pushButtonClose->setEnabled(false);
    }
}

void TestHardware::onItemChecked(bool checked)
{
    Q_UNUSED(checked)

    int checkedCount = 0;
    for (int i = 0; i < m_testItems.size(); ++i) {
        AbstractTestItem *testItem = m_testItems.at(i);
        if (testItem->isChecked()) {
            checkedCount++;
        }
    }

    if (checkedCount == m_testItems.size()) {
        ui->checkBoxAll->setCheckState(Qt::Checked);
    } else if (checkedCount == 0) {
        ui->checkBoxAll->setCheckState(Qt::Unchecked);
    } else {
        ui->checkBoxAll->setCheckState(Qt::PartiallyChecked);
    }
}

void TestHardware::onWizardNext()
{
    for (int i = m_currentTestIndex + 1; i < m_testItems.size(); ++i) {
        AbstractTestItem *item = m_testItems.at(i);
        if (item->isChecked()) {
            item->startTest();
            return;
        }
    }
    //测试结束
    m_testWizard->close();
    //
    ui->groupBoxTest->setEnabled(true);
    ui->pushButtonStart->setEnabled(true);
    ui->pushButtonClose->setEnabled(true);
    //
    onShowMessage(Qt::black, QString("=========== 测试结束 ==========="));
}

void TestHardware::onWizardCancel()
{
    //测试取消
    m_testWizard->close();
    //
    ui->groupBoxTest->setEnabled(true);
    ui->pushButtonStart->setEnabled(true);
    ui->pushButtonClose->setEnabled(true);
    //
    onShowMessage(Qt::black, QString("=========== 测试取消 ==========="));
}

void TestHardware::on_pushButtonStart_clicked()
{
    ui->pushButtonStart->setAttribute(Qt::WA_UnderMouse, false);

    m_currentTestIndex = -1;
    onWizardNext();
}

void TestHardware::on_pushButtonClose_clicked()
{
    close();
    ms_system("killall -9 fio");
    ms_system("fio &");
}

void TestHardware::on_checkBoxAll_clicked(bool checked)
{
    for (int i = 0; i < m_testItems.size(); ++i) {
        AbstractTestItem *testItem = m_testItems.at(i);
        testItem->setChecked(checked);
    }
}

void TestHardware::on_comboBoxOutput_activated(int index)
{
    bool multiEnable = (index == 0);
    if (multiEnable) {
        ShowMessageBox("双击鼠标滚轮可切换主辅屏控制！");
    }
}

void TestHardware::on_comboBoxHDMI2_activated(int index)
{
    bool enable = (index == 1);
    if (enable && !pDisplayDb->sub_enable) {
        ui->comboBoxScreen2->setEnabled(true);
    } else {
        ui->comboBoxScreen2->setEnabled(false);
    }
    if (enable) {
        ShowMessageBox("双击鼠标滚轮可切换主辅屏控制！");
    }
}

void TestHardware::on_pushButtonApply_clicked()
{
    int subEnableChanged = 0, isChanged = 0;
    int screen2Enable = ui->comboBoxHDMI2->currentData().toInt();
    if (pDisplayDb->sub_enable != screen2Enable) {
        pDisplayDb->sub_enable = screen2Enable;
        subEnableChanged = 1;
        isChanged = 1;
        ui->comboBoxScreen2->setEnabled(false);
    }

    int screen1Resolution = ui->comboBoxScreen1->currentData().toInt();
    int screen2Resolution = ui->comboBoxScreen2->currentData().toInt();
    if (pDisplayDb->main_resolution != screen1Resolution || pDisplayDb->sub_resolution != screen2Resolution) {
        setMainSubResolution(screen1Resolution, screen2Resolution, pDisplayDb->spot_resolution);
#if defined(_HI3536A_)
        int timeout = 30;
#else
        int timeout = 20;
#endif
        int result = MessageBox::timeoutQuestion(this, "改变分辨率", "确认改变分辨率?", timeout);
        if (result == MessageBox::Yes) {
            pDisplayDb->main_resolution = screen1Resolution;
            pDisplayDb->sub_resolution = screen2Resolution;
            isChanged = 1;
        } else {
            setMainSubResolution(pDisplayDb->main_resolution, pDisplayDb->sub_resolution, pDisplayDb->spot_resolution);
            ui->comboBoxScreen1->setCurrentIndexFromData(pDisplayDb->main_resolution);
            ui->comboBoxScreen2->setCurrentIndexFromData(pDisplayDb->sub_resolution);
        }
        ui->comboBoxScreen1->setFocus();
        sendMessageOnly(REQUEST_FLAG_SET_SCREEN_END, (void *)&isChanged, sizeof(isChanged));
    }
    if (isChanged) {
        qMsNvr->writeDisplayInfo(pDisplayDb);

        //开启、关闭辅屏
        if (subEnableChanged) {
            REFRESH_E enRefresh;
            if (pDisplayDb->sub_enable == 0) {
                enRefresh = REFRESH_CLEAR_VDEC;
                sendMessage(REQUEST_FLAG_ENABLE_SCREEN, (void *)&enRefresh, sizeof(REFRESH_E));

                SubControl::instance()->setSubEnable(false);
            } else {
                SubControl::instance()->setSubEnable(true);
            }
        }
    }

    bool homologousRboot = false;
    if (ui->comboBoxOutput->isVisible()) {
        int homologous = ui->comboBoxOutput->currentIndex();
        qDebug() << "----previous homologous:" << m_homologous;
        qDebug() << "----current homologous:" << homologous;
        if (homologous != m_homologous) {
            if (MessageBox::Yes == MessageBox::question(this, "修改会在重启后生效，是否立即重启？")) {
                if (homologous == 0) {
                    pDisplayDb->sub_enable = 1;
                } else {
                    pDisplayDb->sub_enable = 0;
                }
                qMsNvr->writeDisplayInfo(pDisplayDb);

                //
                set_param_int(SQLITE_FILE_NAME, PARAM_HOMOLOGOUS, homologous);
                //
                qMsNvr->setQuickSwitchScreenEnable(!homologous);
                homologousRboot = true;
            }
        }
    }
    if (homologousRboot) {
        qMsNvr->reboot();
        qMsApp->setAboutToReboot(true);
    }
    // 临时解决切换分辨率后某些界面无法获取焦点导致不能弹出软键盘的问题
    //MsWaitting::showGlobalWait();
    //MsWaitting::closeGlobalWait();
}

void TestHardware::on_pushButtonMonitorUpdate_clicked()
{
#if defined(_HI3536A_)
    QString msg = "cat /proc/umap/hdmi1_sink";
#else
    QString msg = "cat /proc/umap/hdmi0_sink";
#endif
    ui->labelMonitorInfo1->setText(updateMonitorInfo(msg));
}

#include "settingcontent.h"
#include "MsDevice.h"
#include "ui_settingcontent.h"
#include "AutoLogout.h"
#include "DiskSmartTest.h"
#include "MessageBox.h"
#include "ModuleEvent.h"
#include "ModuleRetrieve.h"
#include "ModuleSettings.h"
#include "ModuleSmartAnalysis.h"
#include "ModuleStatus.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "MyFileSystemDialog.h"
#include "SettingTimeoutTip.h"
#include "autotest.h"
#include "ModuleCamera.h"
#include "centralmessage.h"
#include "commonvideo.h"
#include "mainwindow.h"
#include "msuser.h"
#include "networkcommond.h"
#include "storage.h"
#include "userlogin.h"
#include <QDesktopWidget>
#include <QFile>
#include <QListWidgetItem>
#include <QMouseEvent>
#include <QPainter>
#include <QRegion>
#include "TestHardware.h"

SettingContent *SettingContent::s_settingContent = nullptr;

SettingContent::SettingContent(QWidget *parent)
    : BaseWidget(parent)
    , ui(new Ui::SettingContent)
{
    ui->setupUi(this);

    QFile file(":/style/style/settingstyle.qss");
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        setStyleSheet(file.readAll());
    } else {
        qWarning() << QString("SettingContent load style failed.");
    }
    file.close();

    s_settingContent = this;

    //
    ui->toolButton_liveview->setPixmap(QPixmap(":/mainmenu/mainmenu/preview.png"));
    ui->toolButton_liveview->setHoverPixmap(QPixmap(":/mainmenu/mainmenu/preview_blue.png"));
    ui->toolButton_liveview->setPressedPixmap(QPixmap(":/mainmenu/mainmenu/preview.png"));

    //
    m_menu = new SettingMenu(this);
    connect(m_menu, SIGNAL(settingMenu(MainMenu::MenuItem)), this, SLOT(onSettingMenu(MainMenu::MenuItem)));

    //
    connect(ui->settingListWidget, SIGNAL(itemClicked(SettingItemInfo)), this, SLOT(onSettingItemClicked(SettingItemInfo)));

    //setting page timeout
    m_timerSettingTimeout = new QTimer(this);
    m_timerSettingTimeout->setInterval(1000);
    connect(m_timerSettingTimeout, SIGNAL(timeout()), this, SLOT(onSettingTimeout()));
    initializeSettingTimeout();

    //
    ui->label_mainTitle->installEventFilter(this);
    m_clickTimer = new QTimer(this);
    m_clickTimer->setInterval(1000);
    m_clickTimer->setSingleShot(true);
    connect(m_clickTimer, SIGNAL(timeout()), this, SLOT(onClickTimer()));

    //language
    connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
    onLanguageChanged();
}

SettingContent::~SettingContent()
{
    qDebug() << "SettingContent::~SettingContent()";
    s_settingContent = nullptr;
    delete ui;
}

SettingContent *SettingContent::instance()
{
    return s_settingContent;
}

void SettingContent::dealMessage(MessageReceive *message)
{
    if (m_setting) {
        m_setting->dealMessage(message);
    }
}

void SettingContent::goSetting(const MainMenu::MenuItem &item, int subItem)
{
    gCameraData.getAllIpcData();

    QString mainTitle;
    switch (item) {
    case MainMenu::ItemRetrieve:
        m_setting = new ModuleRetrieve(this);
        mainTitle = GET_TEXT("MENU/10014", "Retrieve");
        break;
    case MainMenu::ItemCamera:
        NetworkCommond::instance()->setPageMode(MOD_CAMERA);
        m_setting = new ModuleCamera(this);
        mainTitle = GET_TEXT("CHANNELMANAGE/30000", "Camera Settings");
        break;
    case MainMenu::ItemSmart:
        NetworkCommond::instance()->setPageMode(MOD_LAYOUT);
        m_setting = new ModuleSmartAnalysis(this);
        mainTitle = GET_TEXT("ANPR/103054", "Smart Analysis");
        break;
    case MainMenu::ItemStorage:
        NetworkCommond::instance()->setPageMode(MOD_RECORD);
        m_setting = new Storage(this);
        mainTitle = GET_TEXT("RECORDADVANCE/91019", "Storage");
        break;
    case MainMenu::ItemEvent:
        NetworkCommond::instance()->setPageMode(MOD_EVENT);
        m_setting = new ModuleEvent(this);
        mainTitle = GET_TEXT("VIDEOLOSS/50000", "Event Settings");
        break;
    case MainMenu::ItemStatus:
        NetworkCommond::instance()->setPageMode(MAD_STATUS);
        m_setting = new ModuleStatus(this);
        mainTitle = GET_TEXT("MENU/10006", "Status");
        break;
    case MainMenu::ItemSettings:
        NetworkCommond::instance()->setPageMode(MOD_SYSTEM);
        m_setting = new ModuleSettings(this);
        mainTitle = GET_TEXT("SYSTEMGENERAL/70000", "System Settings");
        break;
    default:
        return;
    }

    setGeometry(qApp->desktop()->geometry());
    raise();
    show();

    connect(m_setting, SIGNAL(sig_close()), this, SLOT(settingClosed()));
    connect(m_setting, SIGNAL(updateVideoGeometry()), this, SLOT(update()));
    ui->gridLayout->addWidget(m_setting);
    m_setting->initializeSetting();

    const QList<SettingItemInfo> &itemList = m_setting->itemList();
    ui->settingListWidget->setItemList(itemList);

    setGeometry(qApp->desktop()->geometry());
    raise();
    show();

    ui->label_mainTitle->setText(mainTitle);
    if (subItem == -1) {
        ui->settingListWidget->selectFirstItem();
    } else {
        int orderIndex = 0;
        for (int i = 0; i < itemList.count(); i++) {
            if (itemList.at(i).id == subItem) {
                orderIndex = i;
                break;
            }
        }
        ui->settingListWidget->selectItem(orderIndex);
    }
}

void SettingContent::initializeSettingTimeout()
{
    char tmp[20] = { 0 };
    get_param_value(SQLITE_FILE_NAME, PARAM_GUI_MENU_TIME_OUT, tmp, sizeof(tmp), "");
    int menu_timeout = atoi(tmp);
    if (menu_timeout > 0) {
        qDebug() << QString("Settings Page Timeout: %1s").arg(menu_timeout);
        m_settingTimeoutCount = 0;
        m_settingTimeoutInterval = menu_timeout;
        m_timerSettingTimeout->start();
        updateTimeoutTipValue();
    } else {
        m_timerSettingTimeout->stop();
        clearTimeoutTipValue();
    }
}

void SettingContent::refreshTimeout()
{
    if (m_timerSettingTimeout->isActive()) {
        m_settingTimeoutCount = 0;
        updateTimeoutTipValue();
    }
}

void SettingContent::dealMouseMove(const QPoint &pos)
{
    Q_UNUSED(pos)
}

void SettingContent::closeSetting()
{
    if (m_setting) {
        QWidget *widget = qApp->activeWindow();
        if (widget != MainWindow::instance()) {
            BaseDialog *dialog = (BaseDialog*)widget;
            dialog->reject();
        }
    }
    if (removeSetting()) {
        close();
    }
    qMsNvr->mallocTrimLater(1000);
}

void SettingContent::closeToLiveView()
{
    closeSetting();
    emit closed();
}

QPoint SettingContent::globalDownloadPos()
{
    return ui->widgetDownload->mapToGlobal(ui->widgetDownload->rect().center());
}

QRect SettingContent::globalDownloadRect()
{
    QRect rc(ui->widgetDownload->mapToGlobal(ui->widgetDownload->rect().topLeft()), ui->widgetDownload->size());
    return rc;
}

QRect SettingContent::globalContentRect() const
{
    return QRect(ui->widget_content->mapToGlobal(QPoint(0, 0)), ui->widget_content->size());
}

void SettingContent::showCameraManagenment()
{
    goSetting(MainMenu::ItemCamera, ModuleCamera::ItemManagenment);
}

void SettingContent::showDiskSetting()
{
    goSetting(MainMenu::ItemStorage, Storage::ItemDisk);
}

void SettingContent::showLayoutSetting()
{
    goSetting(MainMenu::ItemSettings, ModuleSettings::ItemLayout);
}

bool SettingContent::canAutoLogout()
{
    if (m_setting) {
        //前面有弹窗
        QWidget *widget = qApp->activeWindow();
        if (widget != MainWindow::instance()) {
            qMsCDebug("qt_auto_logout") << "active window:" << widget << ", return";
            return false;
        }
        if (m_setting->canAutoLogout()) {
            return true;
        }
    }
    return false;
}

NetworkResult SettingContent::dealNetworkCommond(const QString &commond)
{
    qDebug() << "SettingContent::dealNetworkCommond," << commond;

    NetworkResult result = NetworkReject;
    if (commond.startsWith("R_Click")) {
        //前面有弹窗
        QWidget *widget = qApp->activeWindow();
        qDebug() << "SettingContent::dealNetworkCommond, activeWindow:" << widget;
        if (widget != MainWindow::instance()) {
            result = NetworkAccept;
        } else {
            settingClosed();
            result = NetworkAccept;
        }
    } else if (commond.startsWith("ChangeFocus_")) {
        result = networkTab2();
    } else if (commond.startsWith("ChangeFocus_Prev")) {
        result = networkTab2_Prev();
    } else if (commond.startsWith("Dial_Insid_Add")) {
        result = networkTab2();
    } else if (commond.startsWith("Dial_Insid_Sub")) {
        result = networkTab2_Prev();
    }
    return result;
}

/**
 * @brief SettingContent::networkTab1
 * 网络键盘：T1
 */
void SettingContent::networkTab1()
{
    qDebug() << "====SettingContent::networkTab1====";
    if (!isVisible()) {
        return;
    }

    //前面有弹窗
    QWidget *widget = qApp->activeWindow();
    if (widget != MainWindow::instance()) {
        return;
    }

    if (ui->toolButton_liveview->hasFocus()) {
        ui->toolButton_liveview->clearFocus();
        qDebug() << "----next:" << 1;
        ui->settingListWidget->selectNextItem();
    } else if (ui->settingListWidget->isLastItemSelected()) {
        qDebug() << "----liveview:" << 1;
        ui->toolButton_liveview->setFocus();
    } else {
        qDebug() << "----next:" << 2;
        ui->settingListWidget->selectNextItem();
    }
}

void SettingContent::networkTab1_Prev()
{
    if (!isVisible()) {
        return;
    }

    //前面有弹窗
    QWidget *widget = qApp->activeWindow();
    if (widget != MainWindow::instance()) {
        return;
    }

    if (ui->toolButton_liveview->hasFocus()) {
        ui->toolButton_liveview->clearFocus();
        ui->settingListWidget->selectPreviousItem();
    } else if (ui->settingListWidget->isFirstItemSelected()) {
        ui->toolButton_liveview->setFocus();
    } else {
        ui->settingListWidget->selectPreviousItem();
    }
}

NetworkResult SettingContent::networkTab2()
{
    if (!isVisible()) {
        return NetworkReject;
    }
    focusNextChild();
    return NetworkAccept;
}

NetworkResult SettingContent::networkTab2_Prev()
{
    if (!isVisible()) {
        return NetworkReject;
    }
    focusPreviousChild();
    return NetworkAccept;
}

QList<QWidget *> SettingContent::test_itemButtonList()
{
    return ui->settingListWidget->test_itemButtonList();
}

bool SettingContent::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->label_mainTitle) {
        switch (event->type()) {
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonDblClick:
            m_clickCount++;
            m_clickTimer->start();
            if (m_clickCount >= 5) {
                if (m_setting->settingType() == BaseSetting::TypeSettings) {
                    //
                    if (qMsNvr->isAudioOpen()) {
                        qMsNvr->closeAudio();
                    }
                    if (qMsNvr->isCamTalkbackOpen()) {
                        qMsNvr->closeTalkback();
                    }
                    //
                    m_clickCount = 0;
                    MainWindow::instance()->hide();
                    TestHardware::instance()->show();
                }
            }
            break;
        default:
            break;
        }
    }
    return BaseWidget::eventFilter(obj, event);
}

void SettingContent::showEvent(QShowEvent *event)
{
    m_isAboutToClose = false;
    setFocus();
    BaseWidget::showEvent(event);
}

void SettingContent::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    QRegion region = QRegion(rect()) - QRegion(CommonVideo::qtVideoGeometry());
    QPainterPath painterPath;
    painterPath.addRegion(region);
    painter.fillPath(painterPath, QColor(255, 255, 255));
}

void SettingContent::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        m_isAboutToClose = true;
        qDebug() << "SettingContent::mousePressEvent, begin close setting.";
        settingClosed();
        qDebug() << "SettingContent::mousePressEvent, end close setting.";
        event->accept();
    } else {
        QWidget::mousePressEvent(event);
    }
}

void SettingContent::escapePressed()
{
    on_toolButton_liveview_clicked();
}

void SettingContent::returnPressed()
{
    QAbstractButton *button = qobject_cast<QAbstractButton *>(focusWidget());
    if (button) {
        qDebug() << "SettingContent::returnPressed," << button;
        button->click();
    }
}

bool SettingContent::isAddToVisibleList()
{
    return true;
}

bool SettingContent::removeSetting()
{
    if (m_setting) {
        if (m_setting->isCloseable()) {
            CommonVideo::stopAllVideo();
            m_setting->closeSetting();
            ui->gridLayout->removeWidget(m_setting);
            m_setting->deleteLater();
            m_setting = nullptr;
            return true;
        }
    }
    return false;
}

void SettingContent::updateTimeoutTipValue()
{
    if (SettingTimeoutTip::instance()) {
        SettingTimeoutTip::instance()->setValue(m_settingTimeoutInterval - m_settingTimeoutCount);
    }
}

void SettingContent::clearTimeoutTipValue()
{
    if (SettingTimeoutTip::instance()) {
        SettingTimeoutTip::instance()->clearValue();
    }
}

void SettingContent::onSettingMenu(const MainMenu::MenuItem &item)
{
    //Menu Authentication
    if (MsUser::isNeedMenuAuthentication()) {
        UserLogin login(MainWindow::s_mainWindow);
        const int &result = login.execLogin();
        if (result != UserLogin::Accepted) {
            return;
        }
    }
    //
    if (!removeSetting()) {
        return;
    }

    switch (item) {
    case MainMenu::ItemPlayback: {
        close();
        MainWindow::s_mainWindow->menuItemClicked(item);
        break;
    }
    case MainMenu::ItemLogout: {
        close();
        emit closed();
        MainWindow::s_mainWindow->menuItemClicked(item);
        break;
    }
    default:
        goSetting(item);
        break;
    }
}

void SettingContent::onLanguageChanged()
{
    if (m_setting) {
        const QList<SettingItemInfo> &itemList = m_setting->itemList();
        //
        ui->settingListWidget->updateLanguage(itemList);
        //小标题
        ui->label_subTitle->setText(ui->settingListWidget->currentItemText());
        //大标题
        switch (m_setting->settingType()) {
        case BaseSetting::TypeRetrueve:
            ui->label_mainTitle->setText(GET_TEXT("MENU/10014", "Retrieve"));
            break;
        case BaseSetting::TypeLayout:
            ui->label_mainTitle->setText(GET_TEXT("MENU/10001", "Layout"));
            break;
        case BaseSetting::TypeCamera:
            ui->label_mainTitle->setText(GET_TEXT("CHANNELMANAGE/30000", "Camera Settings"));
            break;
        case BaseSetting::TypeStorage:
            ui->label_mainTitle->setText(GET_TEXT("RECORDADVANCE/91019", "Storage"));
            break;
        case BaseSetting::TypeEvent:
            ui->label_mainTitle->setText(GET_TEXT("VIDEOLOSS/50000", "Event Settings"));
            break;
        case BaseSetting::TypeSettings:
            ui->label_mainTitle->setText(GET_TEXT("SYSTEMGENERAL/70000", "System Settings"));
            break;
        case BaseSetting::TypeStatus:
            ui->label_mainTitle->setText(GET_TEXT("MENU/10006", "Status"));
            break;
        default:
            break;
        }
    }
    ui->toolButton_liveview->setText(GET_TEXT("CHANNELMANAGE/30006", "Live View"));
}

void SettingContent::onSettingTimeout()
{
    if (m_setting && isVisible()) {
        //前面有弹窗
        QWidget *widget = qApp->activeWindow();
        //qDebug() << "SettingContent::onSettingTimeout, activeWindow:" << widget;
        if (widget != MainWindow::instance()) {
            m_settingTimeoutCount = 0;
            updateTimeoutTipValue();
            return;
        }
        if (m_setting->canAutoLogout()) {
            m_settingTimeoutCount++;
            if (m_settingTimeoutCount > m_settingTimeoutInterval) {
                int logoutInterval = gAutoLogout.interval();
                //setting timeout 和 auto logout 一样时，auto logout 优先
                if (m_settingTimeoutInterval != logoutInterval) {
                    settingClosed();
                    m_settingTimeoutCount = 0;
                    gAutoLogout.refreshTimeout();
                }
            }
        } else {
            m_settingTimeoutCount = 0;
        }
        updateTimeoutTipValue();
    }
}

void SettingContent::settingClosed()
{
    if (removeSetting()) {
        close();

        //
        emit closed();
        qMsNvr->mallocTrimLater(100);
    }
}

void SettingContent::on_toolButton_liveview_clicked()
{
    settingClosed();
}

void SettingContent::onSettingItemClicked(const SettingItemInfo &info)
{
    qMsDebug() << "begin" << info;
    if (m_isAboutToClose) {
        qMsDebug() << "return1" << info;
        return;
    }
    if (!m_setting->isChangeable()) {
        ui->settingListWidget->editItem(m_setting->currentItem());
        qMsDebug() << "return2" << info;
        return;
    }
    m_setting->closeCurrentPage();
    //关闭video小窗口
    CommonVideo::stopAllVideo();
    CommonVideo::hideVideo();
    update();
    //
    ui->label_subTitle->setText(info.name);
    //
    if (!gMsUser.checkBasicPermission(info.mode, info.permission)) {
        m_setting->hide();
        if (!m_labelNote) {
            m_labelNote = new NormalLabel(ui->widget_content);
            m_labelNote->setAlignment(Qt::AlignCenter);
            m_labelNote->setStyleSheet("font-size: 30px;");
        }
        m_labelNote->setGeometry(ui->widget_content->rect());
        m_labelNote->setText(GET_TEXT("COMMON/1011", "Insufficient User Permissions."));
        m_labelNote->show();
        qMsDebug() << "return3" << info;
        return;
    }
    if (m_labelNote) {
        m_labelNote->hide();
    }
    //
    if (!m_setting->isVisible()) {
        m_setting->show();
    }
    m_setting->setCurrentItem(info.id);
    qMsDebug() << "end" << info;
}

void SettingContent::on_toolButton_menu_clicked()
{
    ui->toolButton_menu->setAttribute(Qt::WA_UnderMouse, false);

    QPoint p = ui->toolButton_menu->pos();
    m_menu->move(p.x() - 20, p.y() + 80);
    m_menu->show();
}

void SettingContent::onClickTimer()
{
    m_clickCount = 0;
}

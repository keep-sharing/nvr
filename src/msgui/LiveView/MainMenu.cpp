#include "MainMenu.h"
#include "ui_MainMenu.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "autotest.h"
#include "mainwindow.h"
#include "msuser.h"
#include "userlogin.h"
#include <QDesktopWidget>
#include <QPropertyAnimation>
#include <QtDebug>

MainMenu *MainMenu::self = nullptr;
MainMenu::MainMenu(QWidget *parent)
    : BaseWidget(parent)
    , ui(new Ui::MainMenu)
{
    ui->setupUi(this);
    self = this;

    m_animation = new QPropertyAnimation(this, "pos");
    connect(m_animation, SIGNAL(finished()), this, SLOT(animationFinished()));
    m_animation->setDuration(0);

    QDesktopWidget *desktop = qApp->desktop();
    QRect screenRect = desktop->screenGeometry();
    m_leaveRect = QRect(screenRect.width() / 4 * 3, screenRect.top(), screenRect.width() / 4, screenRect.height());
    m_enterRect = QRect(screenRect.right() - 20, screenRect.top(), 22, screenRect.height() - 100);

    m_items.append(ui->toolButton_playback);
    m_items.append(ui->toolButton_retrieve);
    m_items.append(ui->toolButton_smart);
    m_items.append(ui->toolButton_camera);
    m_items.append(ui->toolButton_storage);
    m_items.append(ui->toolButton_event);
    m_items.append(ui->toolButton_setting);
    m_items.append(ui->toolButton_status);
    m_items.append(ui->toolButton_logout);
    for (int i = 0; i < m_items.size(); ++i) {
        MyToolButton *button = m_items.at(i);
        connect(button, SIGNAL(mouseEnter()), this, SLOT(onToolButtonMouseEnter()));
    }

    connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
    onLanguageChanged();

    //热备模式移除Retrieve、Camera
    if (qMsNvr->isSlaveMode()) {
        ui->toolButton_retrieve->hide();
        ui->toolButton_smart->hide();
        ui->toolButton_camera->hide();
    }

    resize(m_width, screenRect.height() - 100);
}

MainMenu::~MainMenu()
{
    self = nullptr;
    delete ui;
    qDebug() << "MainMenu::~MainMenu()";
}

MainMenu *MainMenu::instance()
{
    return self;
}

void MainMenu::showLogout()
{
    on_toolButton_logout_clicked();
}

void MainMenu::setTempShow()
{
    m_tempShow = true;
    animateShow();
}

void MainMenu::showOrHide(const QPoint &point)
{
    if (m_tempShow) {
        if (isMouseEnter(point)) {
            m_tempShow = false;
        }
        return;
    }

    if (m_show) {
        if (isMouseLeave(point)) {
            animateHide();
        }
    } else {
        if (isMouseEnter(point)) {
            m_tempShow = false;
            animateShow();
        }
    }
}

bool MainMenu::isShow()
{
    return m_show;
}

bool MainMenu::isMouseEnter(const QPoint &mousePos)
{
    return m_enterRect.contains(mousePos);
}

bool MainMenu::isMouseLeave(const QPoint &mousePos)
{
    return !m_leaveRect.contains(mousePos);
}

void MainMenu::dealMenuClicked(const MainMenu::MenuItem &item)
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
    emit itemClicked(item);
}

void MainMenu::onLanguageChanged()
{
    ui->label_view->setText(GET_TEXT("MENU/10009", "VIEW"));
    ui->label_management->setText(GET_TEXT("MENU/10010", "MANAGEMENT"));
    ui->label_system->setText(GET_TEXT("MENU/10011", "SYSTEM"));

    ui->toolButton_playback->setText(GET_TEXT("MENU/10002", "Playback"));
    ui->toolButton_retrieve->setText(GET_TEXT("MENU/10014", "Retrieve"));
    ui->toolButton_smart->setText(GET_TEXT("ANPR/103054", "Smart Analysis"));
    ui->toolButton_camera->setText(GET_TEXT("MENU/10003", "Camera"));
    ui->toolButton_storage->setText(GET_TEXT("RECORDADVANCE/91019", "Storage"));
    ui->toolButton_event->setText(GET_TEXT("MENU/10005", "Event"));
    ui->toolButton_setting->setText(GET_TEXT("MENU/10007", "Settings"));
    ui->toolButton_status->setText(GET_TEXT("MENU/10006", "Status"));
    ui->toolButton_logout->setText(GET_TEXT("MENU/10012", "Logout"));
}

void MainMenu::animateShow()
{
    if (m_show) {
        return;
    }

    QDesktopWidget *desktop = qApp->desktop();
    QRect screenRect = desktop->screenGeometry();
    m_animation->setStartValue(QPoint(screenRect.right() + 1, screenRect.top()));
    m_animation->setEndValue(QPoint(screenRect.right() - m_width + 1, screenRect.top()));
    m_animation->start();

    m_show = true;
    show();
    raise();
}

void MainMenu::animateHide()
{
    m_tempShow = false;
    if (!m_show) {
        return;
    }

    QDesktopWidget *desktop = qApp->desktop();
    QRect screenRect = desktop->screenGeometry();
    m_animation->setStartValue(QPoint(screenRect.right() - m_width + 1, screenRect.top()));
    m_animation->setEndValue(QPoint(screenRect.right() + 1, screenRect.top()));
    m_animation->start();

    m_show = false;
}

void MainMenu::focusPreviousChild()
{
    qDebug() << "MainMenu::focusPreviousChild";
    if (focusWidget() == ui->toolButton_playback) {
        ui->toolButton_logout->setFocus();
        return;
    }
    QWidget::focusPreviousChild();
}

void MainMenu::focusNextChild()
{
    qDebug() << "MainMenu::focusNextChild";
    if (focusWidget() == ui->toolButton_logout) {
        ui->toolButton_playback->setFocus();
        return;
    }
    QWidget::focusNextChild();
}

QWidget *MainMenu::test_randomMenuButton()
{
    int index = qrand() % 8;
    switch (index) {
    case 0:
        return ui->toolButton_playback;
    case 1:
        return ui->toolButton_retrieve;
    case 2:
        return ui->toolButton_smart;
    case 3:
        return ui->toolButton_camera;
    case 4:
        return ui->toolButton_storage;
    case 5:
        return ui->toolButton_event;
    case 6:
        return ui->toolButton_setting;
    case 7:
        return ui->toolButton_status;
    default:
        return nullptr;
    }
}

void MainMenu::returnPressed()
{
    QToolButton *button = qobject_cast<QToolButton *>(focusWidget());
    if (button) {
        qDebug() << "MainMenu::returnPressed," << button;
        button->click();
    }
}

bool MainMenu::isAddToVisibleList()
{
    return true;
}

NetworkResult MainMenu::deal_Dial_Insid_Add()
{
    focusNextChild();
    return NetworkAccept;
}

NetworkResult MainMenu::deal_Dial_Insid_Sub()
{
    focusPreviousChild();
    return NetworkAccept;
}

void MainMenu::animationFinished()
{
    if (!m_show) {
        hide();
    }
}

void MainMenu::onToolButtonMouseEnter()
{
    MyToolButton *sendButton = static_cast<MyToolButton *>(sender());
    for (int i = 0; i < m_items.size(); ++i) {
        MyToolButton *button = m_items.at(i);
        if (button != sendButton) {
            if (button->testAttribute(Qt::WA_UnderMouse)) {
                button->clearUnderMouse();
                button->update();
            }
        }
    }
}

void MainMenu::on_toolButton_playback_clicked()
{
    dealMenuClicked(ItemPlayback);
}

void MainMenu::on_toolButton_retrieve_clicked()
{
    dealMenuClicked(ItemRetrieve);
}

void MainMenu::on_toolButton_smart_clicked()
{
    dealMenuClicked(ItemSmart);
}

void MainMenu::on_toolButton_camera_clicked()
{
    dealMenuClicked(ItemCamera);
}

void MainMenu::on_toolButton_storage_clicked()
{
    dealMenuClicked(ItemStorage);
}

void MainMenu::on_toolButton_event_clicked()
{
    dealMenuClicked(ItemEvent);
}

void MainMenu::on_toolButton_setting_clicked()
{
    dealMenuClicked(ItemSettings);
}

void MainMenu::on_toolButton_status_clicked()
{
    dealMenuClicked(ItemStatus);
}

void MainMenu::on_toolButton_logout_clicked()
{
    dealMenuClicked(ItemLogout);
}

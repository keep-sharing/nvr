#include "mswizard.h"
#include "ui_mswizard.h"
#include "MyDebug.h"
#include "abstractwizardpage.h"
#include "centralmessage.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "networkcommond.h"
#include "SubControl.h"
#include "wizardpageactivate.h"
#include "wizardpagecamera.h"
#include "wizardpagecloud.h"
#include "wizardpagedisk.h"
#include "wizardpagenetwork.h"
#include "wizardpagep2p.h"
#include "wizardpagepattern.h"
#include "wizardpagequestion.h"
#include "wizardpagerecord.h"
#include "wizardpagetime.h"
#include "wizardpageuser.h"
#include <QDesktopWidget>
#include <QFile>
#include <QPainter>
#include <QtDebug>

MsWizard *MsWizard::s_msWizard = nullptr;

MsWizard::MsWizard(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MsWizard)
{
    ui->setupUi(this);
    setFocusPolicy(Qt::StrongFocus);
    s_msWizard = this;

    //
    QFile file(":/style/style/wizardstyle.qss");
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        setStyleSheet(file.readAll());
    } else {
        qWarning() << QString("MsWizard load style failed.");
    }
    file.close();

    //
    m_buttonGroup = new QButtonGroup(this);
    m_buttonGroup->addButton(ui->toolButton_1, 2);
    m_buttonGroup->addButton(ui->toolButton_2, 3);
    m_buttonGroup->addButton(ui->toolButton_3, 4);
    m_buttonGroup->addButton(ui->toolButton_4, 5);
    m_buttonGroup->addButton(ui->toolButton_5, 6);
    m_buttonGroup->addButton(ui->toolButton_6, 7);
    m_buttonGroup->addButton(ui->toolButton_7, 8);

    m_activateButtonGroup = new QButtonGroup(this);
    m_activateButtonGroup->addButton(ui->toolButton_a1, 0);
    m_activateButtonGroup->addButton(ui->toolButton_a2, 1);
    m_activateButtonGroup->addButton(ui->toolButton_a3, 2);
    m_activateButtonGroup->addButton(ui->toolButton_a4, 3);
    m_activateButtonGroup->addButton(ui->toolButton_a5, 4);
    m_activateButtonGroup->addButton(ui->toolButton_a6, 5);
    m_activateButtonGroup->addButton(ui->toolButton_a7, 6);
    m_activateButtonGroup->addButton(ui->toolButton_a8, 7);
    m_activateButtonGroup->addButton(ui->toolButton_a9, 8);

    //
    connect(ui->pushButton_back, SIGNAL(clicked(bool)), this, SLOT(onPushButtonBackClicked()));
    connect(ui->pushButton_next, SIGNAL(clicked(bool)), this, SLOT(onPushButtonNextClicked()));
    connect(ui->pushButton_skip, SIGNAL(clicked(bool)), this, SLOT(onPushButtonSkipClicked()));

    //
    connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
    onLanguageChanged();

    ui->pushButton_test_clearPassword->hide();
    ui->pushButton_test_clearQuestion->hide();
    //
    hide();
}

MsWizard::~MsWizard()
{
    s_msWizard = nullptr;
    delete ui;
    qDebug() << "MsWizard::~MsWizard()";
}

MsWizard *MsWizard::instance()
{
    return s_msWizard;
}

void MsWizard::showWizard()
{
    qDebug() << "MsWizard::showWizard, 1";
    NetworkCommond::instance()->setPageMode(MOD_WIZARD);

    //是否需要激活
    qDebug() << "MsWizard::showWizard, 2";
    memset(&m_adminUser, 0, sizeof(db_user));
    read_user(SQLITE_FILE_NAME, &m_adminUser, 0);
    //清空密码
    //    snprintf(m_adminUser.password_ex, sizeof(m_adminUser.password_ex), "%s", "");
    //    write_user(SQLITE_FILE_NAME, &m_adminUser);
    //
    qDebug() << "MsWizard::showWizard, 3";
    if (QString(m_adminUser.password_ex).isEmpty()) {
        setWizardMode(ModeActivate);
    } else {
        setWizardMode(ModeNormal);
    }

    //
    qDebug() << "MsWizard::showWizard, 4";
    showWizardPage(m_currentItemType);

    //
    qDebug() << "MsWizard::showWizard, 5";
    char tmp[20] = { 0 };
    get_param_value(SQLITE_FILE_NAME, PARAM_GUI_WIZARD_ENABLE, tmp, sizeof(tmp), "");
    int wizardEnable = atoi(tmp);
    ui->checkBox_enable->setChecked(wizardEnable);

    //
    qDebug() << "MsWizard::showWizard, 6";
    setGeometry(SubControl::instance()->logicalMainScreenGeometry());
    raise();
    show();

    qDebug() << "MsWizard::showWizard, end";
}

void MsWizard::finishWizard()
{
    hide();
    emit sig_finished();
}

void MsWizard::skipWizard()
{
    const int result = MessageBox::question(this, GET_TEXT("WIZARD/11002", "Are you sure to exit the Wizard?"));
    if (result == MessageBox::Yes) {
        hide();
        emit sig_finished();
    }
}

void MsWizard::next()
{
    onPushButtonNextClicked();
}

bool MsWizard::isActivatePage()
{
    return m_currentItemType == Wizard_Activate;
}

void MsWizard::setWizardMode(const WizardMode &mode)
{
    m_mode = mode;

    //保留Wizard的进度
    if (m_mode == ModeActivate) {
        ui->widget_activateState->show();
        ui->widget_normalState->hide();
    } else {
        ui->widget_activateState->hide();
        ui->widget_normalState->show();
        if (m_currentItemType < Wizard_User) {
            m_currentItemType = Wizard_User;
        }
    }
}

WizardMode MsWizard::wizardMode() const
{
    return m_mode;
}

void MsWizard::dealMessage(MessageReceive *message)
{
    Q_UNUSED(message)
}

void MsWizard::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.drawPixmap(rect(), *MsDevice::s_backgroundPixmap);
}

void MsWizard::showEvent(QShowEvent *event)
{
    qMsDebug() << "";
    QWidget::showEvent(event);
}

void MsWizard::hideEvent(QHideEvent *event)
{
    qMsDebug() << "";
    QWidget::hideEvent(event);
}

void MsWizard::showWizardPage(const WizardType &type)
{
    AbstractWizardPage *page = nullptr;
    if (m_pageMap.contains(type)) {
        page = m_pageMap.value(type);
        qMsDebug() << QString("MsWizard::showWizardPage 111 type:[%1]").arg(type);
    } else {
        switch (type) {
        case Wizard_Activate:
            page = new WizardPageActivate(this);
            break;
        case wizard_Pattern:
            page = new WizardPagePattern(this);
            break;
        case Wizard_Question:
            page = new WizardPageQuestion(this);
            break;
        case Wizard_User:
            page = new WizardPageUser(this);
            break;
        case Wizard_Time:
            page = new WizardPageTime(this);
            break;
        case Wizard_NetWork:
            page = new WizardPageNetwork(this);
            break;
        case Wizard_Disk:
            page = new WizardPageDisk(this);
            break;
        case Wizard_Camera:
            page = new WizardPageCamera(this);
            break;
        case Wizard_P2P:
            if (qMsNvr->isMilesight()) {
                page = new WizardPageCloud(this);
            } else {
                page = new WizardPageP2P(this);
            }
            break;
        case Wizard_Record:
            page = new WizardPageRecord(this);
            break;
        default:
            break;
        }
        if (page) {
            m_pageMap.insert(type, page);
        }
        qMsDebug() << QString("MsWizard::showWizardPage 222 type:[%1]").arg(type);
    }
    if (page) {
        QLayoutItem *item = ui->gridLayout_content->itemAtPosition(0, 0);
        if (item) {
            QWidget *widget = item->widget();
            if (widget) {
                widget->hide();
            }
            ui->gridLayout_content->removeItem(item);
            delete item;
        }
        ui->gridLayout_content->addWidget(page, 0, 0);
        page->initializeData();
        page->show();
        qMsDebug() << QString("page page page  m_currentItemType:[%1]  type:[%2]  m_mode:[%3]").arg(m_currentItemType).arg(type).arg(m_mode);
    }
    //
    m_currentItemType = type;
    switch (m_currentItemType) {
    case Wizard_Activate:
        ui->pushButton_back->setEnabled(false);
        ui->pushButton_next->setEnabled(true);
        ui->pushButton_next->setText(GET_TEXT("WIZARD/11000", "Next"));
        ui->pushButton_skip->setEnabled(false);
        ui->toolButton_a1->setChecked(true);
        ui->toolButton_hourglass->setIcon(QIcon(QString(":/wizard/wizard/wizard_a1.png")));
        break;
    case wizard_Pattern:
        ui->pushButton_back->setEnabled(false);
        ui->pushButton_next->setEnabled(true);
        ui->pushButton_next->setText(GET_TEXT("WIZARD/11000", "Next"));
        ui->pushButton_skip->setEnabled(true);
        ui->toolButton_a2->setChecked(true);
        ui->toolButton_hourglass->setIcon(QIcon(QString(":/wizard/wizard/wizard_a2.png")));
        break;
    case Wizard_Question:
        ui->pushButton_back->setEnabled(false);
        ui->pushButton_next->setEnabled(true);
        ui->pushButton_next->setText(GET_TEXT("WIZARD/11000", "Next"));
        ui->pushButton_skip->setEnabled(true);
        ui->toolButton_a3->setChecked(true);
        ui->toolButton_hourglass->setIcon(QIcon(QString(":/wizard/wizard/wizard_a3.png")));
        break;
    case Wizard_User:
        ui->pushButton_back->setEnabled(false);
        ui->pushButton_next->setEnabled(true);
        ui->pushButton_next->setText(GET_TEXT("WIZARD/11000", "Next"));
        ui->pushButton_skip->setEnabled(true);
        ui->toolButton_1->setChecked(true);
        ui->toolButton_8->setIcon(QIcon(QString(":/wizard/wizard/wizard_1.png")));
        break;
    case Wizard_Time:
        if (m_mode == ModeActivate) {
            ui->pushButton_back->setEnabled(false);
        } else {
            ui->pushButton_back->setEnabled(true);
        }
        ui->pushButton_next->setEnabled(true);
        ui->pushButton_next->setText(GET_TEXT("WIZARD/11000", "Next"));
        ui->pushButton_skip->setEnabled(true);
        ui->toolButton_2->setChecked(true);
        ui->toolButton_8->setIcon(QIcon(QString(":/wizard/wizard/wizard_2.png")));
        ui->toolButton_a4->setChecked(true);
        ui->toolButton_hourglass->setIcon(QIcon(QString(":/wizard/wizard/wizard_a4.png")));
        break;
    case Wizard_NetWork:
        ui->pushButton_back->setEnabled(true);
        ui->pushButton_next->setEnabled(true);
        ui->pushButton_next->setText(GET_TEXT("WIZARD/11000", "Next"));
        ui->pushButton_skip->setEnabled(true);
        ui->toolButton_3->setChecked(true);
        ui->toolButton_8->setIcon(QIcon(QString(":/wizard/wizard/wizard_3.png")));
        ui->toolButton_a5->setChecked(true);
        ui->toolButton_hourglass->setIcon(QIcon(QString(":/wizard/wizard/wizard_a5.png")));
        break;
    case Wizard_Disk:
        ui->pushButton_back->setEnabled(true);
        ui->pushButton_next->setEnabled(true);
        ui->pushButton_next->setText(GET_TEXT("WIZARD/11000", "Next"));
        ui->pushButton_skip->setEnabled(true);
        ui->toolButton_4->setChecked(true);
        ui->toolButton_8->setIcon(QIcon(QString(":/wizard/wizard/wizard_4.png")));
        ui->toolButton_a6->setChecked(true);
        ui->toolButton_hourglass->setIcon(QIcon(QString(":/wizard/wizard/wizard_a6.png")));
        break;
    case Wizard_Camera:
        ui->pushButton_back->setEnabled(true);
        ui->pushButton_next->setEnabled(true);
        ui->pushButton_next->setText(GET_TEXT("WIZARD/11000", "Next"));
        ui->pushButton_skip->setEnabled(true);
        ui->toolButton_5->setChecked(true);
        ui->toolButton_8->setIcon(QIcon(QString(":/wizard/wizard/wizard_5.png")));
        ui->toolButton_a7->setChecked(true);
        ui->toolButton_hourglass->setIcon(QIcon(QString(":/wizard/wizard/wizard_a7.png")));
        break;
    case Wizard_P2P:
        ui->pushButton_back->setEnabled(true);
        ui->pushButton_next->setEnabled(true);
        ui->pushButton_next->setText(GET_TEXT("WIZARD/11000", "Next"));
        ui->pushButton_skip->setEnabled(true);
        ui->toolButton_6->setChecked(true);
        ui->toolButton_8->setIcon(QIcon(QString(":/wizard/wizard/wizard_6.png")));
        ui->toolButton_a8->setChecked(true);
        ui->toolButton_hourglass->setIcon(QIcon(QString(":/wizard/wizard/wizard_a8.png")));
        break;
    case Wizard_Record:
        ui->pushButton_back->setEnabled(true);
        ui->pushButton_next->setEnabled(true);
        ui->pushButton_next->setText(GET_TEXT("WIZARD/11001", "Finish"));
        ui->pushButton_skip->setEnabled(true);
        ui->toolButton_7->setChecked(true);
        ui->toolButton_8->setIcon(QIcon(QString(":/wizard/wizard/wizard_7.png")));
        ui->toolButton_a9->setChecked(true);
        ui->toolButton_hourglass->setIcon(QIcon(QString(":/wizard/wizard/wizard_a9.png")));
    default:
        break;
    }
}

void MsWizard::onLanguageChanged()
{
    ui->label_mainTitle->setText(GET_TEXT("WIZARD/11003", "WIZARD"));
    ui->label_subTitle->setText(GET_TEXT("WIZARD/11004", "Network Video Recorder"));
    ui->checkBox_enable->setText(GET_TEXT("WIZARD/11005", "Start Wizard when startup?"));
    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));
    ui->pushButton_skip->setText(GET_TEXT("WIZARD/11006", "Skip"));

    switch (m_currentItemType) {
    case Wizard_Record:
        ui->pushButton_next->setText(GET_TEXT("WIZARD/11001", "Finish"));
        break;
    default:
        ui->pushButton_next->setText(GET_TEXT("WIZARD/11000", "Next"));
        break;
    }
}

void MsWizard::onPushButtonBackClicked()
{
    AbstractWizardPage *page = m_pageMap.value(m_currentItemType, nullptr);
    if (page) {
        page->previousPage();
    }
}

void MsWizard::onPushButtonNextClicked()
{
    AbstractWizardPage *page = m_pageMap.value(m_currentItemType, nullptr);
    if (page) {
        page->nextPage();
    }
}

void MsWizard::onPushButtonSkipClicked()
{
    AbstractWizardPage *page = m_pageMap.value(m_currentItemType, nullptr);
    if (page) {
        page->skipWizard();
    }
}

void MsWizard::on_checkBox_enable_clicked(bool checked)
{
    char tmp[20] = { 0 };
    snprintf(tmp, sizeof(tmp), "%d", checked);
    set_param_value(SQLITE_FILE_NAME, PARAM_GUI_WIZARD_ENABLE, tmp);
}

/**
 * @brief MsWizard::on_pushButton_test_clearPassword_clicked
 * 仅用于测试
 */
void MsWizard::on_pushButton_test_clearPassword_clicked()
{
    db_user user;
    memset(&user, 0, sizeof(db_user));
    read_user(SQLITE_FILE_NAME, &user, 0);
    snprintf(user.password_ex, sizeof(user.password_ex), "%s", QString().toUtf8().data());
    write_user(SQLITE_FILE_NAME, &user);

    ShowMessageBox("admin密码已清除。");
}

/**
 * @brief MsWizard::on_pushButton_test_clearQuestion_clicked
 * 仅用于测试
 */
void MsWizard::on_pushButton_test_clearQuestion_clicked()
{
    struct squestion sqa;
    memset(&sqa, 0x0, sizeof(struct squestion));
    write_encrypted_list(SQLITE_FILE_NAME, &sqa, 1);
    write_encrypted_list(SQLITE_FILE_NAME, &sqa, 2);
    write_encrypted_list(SQLITE_FILE_NAME, &sqa, 3);
    ShowMessageBox("密保已清除。");
}

#include "resetpassword.h"
#include "ui_resetpassword.h"
#include "centralmessage.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include "MsWaitting.h"
#include <QCryptographicHash>
#include <QDesktopWidget>
#include <QtDebug>

extern "C" {
#include "msdb.h"
}

ResetPassword *ResetPassword::s_resetPassword = nullptr;

ResetPassword::ResetPassword(QWidget *parent)
    : BaseShadowDialog(parent)
    , ui(new Ui::ResetPassword)
{
    ui->setupUi(this);
    s_resetPassword = this;

    step = 1;
    m_pattern_text = "0";
    ui->label_note->setText(GET_TEXT("WIZARD/11063", "Please fill in the answers for authentication."));
    m_waitting = new MsWaitting(this);

    ui->lineEdit_answer1->setCheckMode(MyLineEdit::EmptyCheck);
    ui->lineEdit_answer2->setCheckMode(MyLineEdit::EmptyCheck);
    ui->lineEdit_answer3->setCheckMode(MyLineEdit::EmptyCheck);
    ui->lineEdit_password->setCheckMode(MyLineEdit::PasswordCheck);

    connect(ui->widget_pattern, SIGNAL(drawStart()), this, SLOT(onDrawStart()));
    connect(ui->widget_pattern, SIGNAL(drawFinished(QString)), this, SLOT(onDrawFinished(QString)));

    onLanguageChanged();
}

ResetPassword::~ResetPassword()
{
    s_resetPassword = nullptr;
    delete ui;
}

void ResetPassword::onLanguageChanged()
{
    ui->label_question1->setText(GET_TEXT("USER/74071", "Question 1"));
    ui->label_answer1->setText(GET_TEXT("USER/74074", "Answer"));
    ui->label_question2->setText(GET_TEXT("USER/74072", "Question 2"));
    ui->label_answer2->setText(GET_TEXT("USER/74074", "Answer"));
    ui->label_question3->setText(GET_TEXT("USER/74073", "Question 3"));
    ui->label_answer3->setText(GET_TEXT("USER/74074", "Answer"));
    ui->pushButton_next->setText(GET_TEXT("WIZARD/11000", "Next"));
    ui->pushButton_cancel->setText(GET_TEXT("COMMON/1004", "Cancel"));

    ui->label_userName->setText(GET_TEXT("COMMON/1007", "User Name"));
    ui->label_password->setText(GET_TEXT("COMMON/1008", "Password"));
    ui->label_confirm->setText(GET_TEXT("USER/74055", "Confirm Password"));
    ui->pushButton_ok->setText(GET_TEXT("COMMON/1036", "Save"));
    ui->pushButton_cancel_2->setText(GET_TEXT("COMMON/1004", "Cancel"));

    ui->label_tips->setText(GET_TEXT("USER/74076", "Please fill in the password for authentication."));
    ui->label_UserName->setText(GET_TEXT("COMMON/1007", "User Name"));
    ui->label_Password->setText(GET_TEXT("COMMON/1008", "Password"));
    ui->label_patternTips->setText(GET_TEXT("USER/74063", "Please connect at least 4 dots."));
    ui->pushButton_Next->setText(GET_TEXT("WIZARD/11000", "Next"));
    ui->pushButton_OK->setText(GET_TEXT("COMMON/1001", "OK"));
    ui->pushButton_Cancel->setText(GET_TEXT("COMMON/1004", "Cancel"));
    ui->pushButton_Cancel_2->setText(GET_TEXT("COMMON/1004", "Cancel"));
}

ResetPassword *ResetPassword::instance()
{
    return s_resetPassword;
}

bool ResetPassword::initializeData(QString userName)
{
    if (!userName.isEmpty()) {
        qDebug() << QString("ResetPassword::initializeData userName:[%1]").arg(userName);
        initResetUnlockPattern(userName);
        return true;
    }

    memset(m_sqa, 0x0, sizeof(struct squestion) * 3);
    read_encrypted_list(SQLITE_FILE_NAME, m_sqa);

    if (!m_sqa[0].enable || !m_sqa[1].enable || !m_sqa[2].enable) {
        return false;
    }

    ui->label_title->setText(GET_TEXT("WIZARD/11062", "Reset Password"));
    ui->lineEdit_question1->setText(getQuestion(0));
    ui->lineEdit_question2->setText(getQuestion(1));
    ui->lineEdit_question3->setText(getQuestion(2));

    ui->stackedWidget->setCurrentIndex(0);
    return true;
}

void ResetPassword::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_CHECK_SQA_LOCK:
        ON_RESPONSE_FLAG_CHECK_SQA_LOCK(message);
        break;
    case RESPONSE_FLAG_CHECKSQA_LOCK:
        ON_RESPONSE_FLAG_CHECKSQA_LOCK(message);
        break;
    }
}

void ResetPassword::ON_RESPONSE_FLAG_CHECK_SQA_LOCK(MessageReceive *message)
{
    //m_waitting->//closeWait();

    if (!message->data) {
        qWarning() << "ResetPassword::ON_RESPONSE_FLAG_CHECK_SQA_LOCK, data is null.";
        return;
    }

    int result = *(int *)message->data;
    if (result == 0) {
        ui->stackedWidget->setCurrentIndex(1);
    } else {
        sendMessage(REQUEST_FLAG_CHECKSQA_LOCK, nullptr, 0);
        //m_waitting->//showWait();
    }
}

void ResetPassword::ON_RESPONSE_FLAG_CHECKSQA_LOCK(MessageReceive *message)
{
    //m_waitting->//closeWait();

    struct check_sqa_params *params = (struct check_sqa_params *)message->data;
    if (!params) {
        qWarning() << "ResetPassword::ON_RESPONSE_FLAG_CHECKSQA_LOCK, data is null.";
        return;
    }
    if (params->allow_times > 0) {
        ShowMessageBox(GET_TEXT("WIZARD/11072", "Incorrect answer, %1 attempt(s) left.").arg(params->allow_times));
    } else {
        ShowMessageBox(GET_TEXT("WIZARD/11073", "Requests are too frequent, please try again later."));
    }
}

void ResetPassword::showEvent(QShowEvent *event)
{
    QPoint p = QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size(), qApp->desktop()->geometry()).topLeft();
    move(p);
    BaseShadowDialog::showEvent(event);
}

QString ResetPassword::getQuestion(int index)
{
    struct squestion *sqa = nullptr;
    switch (index) {
    case 0:
        sqa = &m_sqa[0];
        break;
    case 1:
        sqa = &m_sqa[1];
        break;
    case 2:
        sqa = &m_sqa[2];
        break;
    }
    QString text;
    if (sqa) {
        switch (sqa->sqtype) {
        case 0:
            text = GET_TEXT("WIZARD/11045", "What's your father's name?");
            break;
        case 1:
            text = GET_TEXT("WIZARD/11046", "What's your favorite sport?");
            break;
        case 2:
            text = GET_TEXT("WIZARD/11047", "What's your mother's name?");
            break;
        case 3:
            text = GET_TEXT("WIZARD/11048", "What's your mobile number?");
            break;
        case 4:
            text = GET_TEXT("WIZARD/11049", "What's your first pet's name?");
            break;
        case 5:
            text = GET_TEXT("WIZARD/11050", "What's your favorite book?");
            break;
        case 6:
            text = GET_TEXT("WIZARD/11051", "What's your favorite game?");
            break;
        case 7:
            text = GET_TEXT("WIZARD/11052", "What's your favorite food?");
            break;
        case 8:
            text = GET_TEXT("WIZARD/11053", "What's your lucky number?");
            break;
        case 9:
            text = GET_TEXT("WIZARD/11054", "What's your favorite color?");
            break;
        case 10:
            text = GET_TEXT("WIZARD/11055", "What's your best friend's name?");
            break;
        case 11:
            text = GET_TEXT("WIZARD/11056", "Where did you go on your first trip?");
            break;
        case SQA_CUSTOMIZED_NO:
            text = QString(sqa->squestion);
            break;
        default:
            break;
        }
    }
    return text;
}

void ResetPassword::on_pushButton_next_clicked()
{
    const QString &answer1 = ui->lineEdit_answer1->text();
    const QString &answer2 = ui->lineEdit_answer2->text();
    const QString &answer3 = ui->lineEdit_answer3->text();
    bool valid = ui->lineEdit_answer1->checkValid();
    valid = ui->lineEdit_answer2->checkValid() && valid;
    valid = ui->lineEdit_answer3->checkValid() && valid;
    if (!valid) {
        return;
    }
    if (answer1 != QString(m_sqa[0].answer) || answer2 != QString(m_sqa[1].answer) || answer3 != QString(m_sqa[2].answer)) {
        sendMessage(REQUEST_FLAG_CHECKSQA_LOCK, nullptr, 0);
    } else {
        sendMessage(REQUEST_FLAG_CHECK_SQA_LOCK, nullptr, 0);
    }
    //m_waitting->//showWait();
}

void ResetPassword::on_pushButton_cancel_clicked()
{
    reject();
}

void ResetPassword::on_pushButton_ok_clicked()
{
    const QString &adminPassword = ui->lineEdit_password->text();
    const QString &adminPasswordMD5 = QCryptographicHash::hash(adminPassword.toLatin1(), QCryptographicHash::Md5).toHex();
    const QString &confirmPassword = ui->lineEdit_confirm->text();
    bool valid = ui->lineEdit_password->checkValid();
    //
    //
    if (!adminPassword.isEmpty() && adminPassword != confirmPassword) {
        ui->lineEdit_confirm->setCustomValid(false, GET_TEXT("MYLINETIP/112008", "Passwords do not match."));
        valid = false;
    }
    if (!valid) {
        return;
    }
    //
    db_user adminUser;
    memset(&adminUser, 0, sizeof(db_user));
    read_user(SQLITE_FILE_NAME, &adminUser, 0);
    snprintf(adminUser.password_ex, sizeof(adminUser.password_ex), "%s", adminPassword.toStdString().c_str());
    snprintf(adminUser.password, sizeof(adminUser.password), "%s", adminPasswordMD5.toStdString().c_str());
    write_user(SQLITE_FILE_NAME, &adminUser);
    sendMessageOnly(REQUEST_FLAG_USER_UPDATE, nullptr, 0);
    //
    if (qMsNvr->isPoe()) {
        const int result = MessageBox::question(this, GET_TEXT("WIZARD/11079", "NVR activated! The NVR password will also be used to activate the PoE channels."));
        if (result == MessageBox::Yes) {
            set_param_value(SQLITE_FILE_NAME, PARAM_POE_PSD, confirmPassword.toUtf8().data());
            sendMessage(REQUEST_FLAG_MODIFY_POE_CAMERA_PWD, nullptr, 0);
        }
    }
    //
    accept();
}

void ResetPassword::on_pushButton_cancel_2_clicked()
{
    reject();
}

void ResetPassword::initResetUnlockPattern(QString userName)
{
    ui->stackedWidget->setCurrentIndex(2);
    m_pattern_text = "0";
    ui->lineEdit_UserName->setText(userName);
    ui->lineEdit_UserName->setEnabled(false);
    ui->label_title->setText(GET_TEXT("USER/74068", "Reset Unlock Pattern"));
}

void ResetPassword::onDrawStart()
{
    m_pattern_text = "0";
    ui->pushButton_OK->setEnabled(false);
    ui->label_patternTips->setText(GET_TEXT("USER/74060", "Release mouse when finished."));
}

void ResetPassword::onDrawFinished(QString text)
{
    qDebug() << QString("ResetPassword onDrawFinished!!!  m_pattern_text:[%1] : [%2]").arg(m_pattern_text).arg(text);
    static QString pattern = "0";
    if (step == 1) {
        if (text.size() < 4) {
            ui->label_patternTips->setText(GET_TEXT("USER/74056", "Connect at least 4 dots, please try again."));
            return;
        }

        step++;
        pattern = text;
        ui->label_patternTips->setText(GET_TEXT("USER/74057", "Please draw again to confirm."));
        return;
    } else {
        step--;
        if (pattern == text) {
            m_pattern_text = pattern;
            ui->label_patternTips->setText(GET_TEXT("USER/74058", "Set unlock pattern successfully."));
            ui->pushButton_OK->setEnabled(true);
        } else {
            ui->label_patternTips->setText(GET_TEXT("USER/74059", "Unmatched pattern, please try again."));
        }
        pattern = "0";
    }
}

void ResetPassword::on_pushButton_Cancel_clicked()
{
    reject();
}

void ResetPassword::on_pushButton_Next_clicked()
{
    QString userName = ui->lineEdit_UserName->text();
    QString password = ui->lineEdit_Password->text();
    if (!ui->lineEdit_Password->checkValid()) {
        return;
    }

    db_user user;
    memset(&user, 0, sizeof(db_user));
    read_user_by_name(SQLITE_FILE_NAME, &user, (char *)userName.toStdString().c_str());

    const QString &passwordMD5 = QCryptographicHash::hash(password.toLatin1(), QCryptographicHash::Md5).toHex();
    if (passwordMD5 != QString(user.password)) {
        ui->lineEdit_Password->setCustomValid(false, GET_TEXT("USER/74069", "Incorrect Password"));
        ui->lineEdit_Password->clear();
        return;
    }
    ui->stackedWidget->setCurrentIndex(3);
    ui->widget_pattern->setEnabled(true);
    ui->widget_pattern->setStyle(GrayStyle);
    ui->pushButton_OK->setEnabled(false);
}

void ResetPassword::on_pushButton_OK_clicked()
{

    QString userName = ui->lineEdit_UserName->text();
    db_user user;
    memset(&user, 0, sizeof(db_user));
    read_user_by_name(SQLITE_FILE_NAME, &user, (char *)userName.toStdString().c_str());
    snprintf(user.pattern_psw, sizeof(user.pattern_psw), "%s", m_pattern_text.toStdString().c_str());
    write_user(SQLITE_FILE_NAME, &user);

    accept();
}

void ResetPassword::on_pushButton_Cancel_2_clicked()
{
    reject();
}

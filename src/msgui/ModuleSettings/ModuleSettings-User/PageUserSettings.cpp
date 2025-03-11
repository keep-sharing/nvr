#include "PageUserSettings.h"
#include "ui_PageUserSettings.h"
#include "AddUserDialog.h"
#include "EditUserDialog.h"
#include "MessageBox.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "centralmessage.h"
#include "mainwindow.h"
#include "msuser.h"
#include <QCryptographicHash>

const int UserInfoRole = Qt::UserRole + 50;

PageUserSettings::PageUserSettings(QWidget *parent)
    : AbstractSettingPage(parent)
    , ui(new Ui::UserSetting)
{
    ui->setupUi(this);
    //
    ui->tabBar->addTab(GET_TEXT("USER/74011", "User"), PageUser);
    ui->tabBar->addTab(GET_TEXT("USER/74015", "Security Question"), PageSecurityQuestion);
    connect(ui->tabBar, SIGNAL(tabClicked(int)), this, SLOT(onTabBarClicked(int)));
    //
    QStringList headerList;
    headerList << "";
    headerList << GET_TEXT("CAMERASEARCH/32002", "NO.");
    headerList << GET_TEXT("COMMON/1007", "User Name");
    headerList << GET_TEXT("USER/74001", "User Level");
    headerList << GET_TEXT("COMMON/1019", "Edit");
    headerList << GET_TEXT("CHANNELMANAGE/30023", "Delete");
    ui->tableView->setHorizontalHeaderLabels(headerList);
    ui->tableView->setColumnCount(headerList.size());
    ui->tableView->setHeaderCheckable(false);
    ui->tableView->hideColumn(ColumnCheck);
    //delegate
    ui->tableView->setItemDelegateForColumn(ColumnEdit, new ItemButtonDelegate(QPixmap(":/common/common/edit.png"), this));
    ui->tableView->setItemDelegateForColumn(ColumnDelete, new ItemButtonDelegate(QPixmap(":/common/common/delete.png"), this));
    //sort
    ui->tableView->setSortingEnabled(false);

    ui->lineEdit_answer1->setCheckMode(MyLineEdit::EmptyCheck);
    ui->lineEdit_answer2->setCheckMode(MyLineEdit::EmptyCheck);
    ui->lineEdit_answer3->setCheckMode(MyLineEdit::EmptyCheck);
    //
    connect(ui->tableView, SIGNAL(itemClicked(int, int)), this, SLOT(onTableItemClicked(int, int)));

    onLanguageChanged();
}

PageUserSettings::~PageUserSettings()
{
    delete ui;
}

void PageUserSettings::initializeData()
{
    ui->tabBar->setCurrentTab(PageUser);

    m_userMap.clear();

    db_user user_array[MAX_USER];
    int userCount = 0;
    read_users(SQLITE_FILE_NAME, user_array, &userCount);
    for (int i = 0; i < userCount; ++i) {
        const db_user &user = user_array[i];
        m_userMap.insert(user.id, user);
    }
    updateTable();
    //
    initializeSecurityQuestion();
}

void PageUserSettings::processMessage(MessageReceive *message)
{
    Q_UNUSED(message)
}

void PageUserSettings::resizeEvent(QResizeEvent *event)
{
    int w = (width() - 50) / 5;
    for (int i = 0; i < 5; ++i) {
        ui->tableView->setColumnWidth(i, w);
    }
    AbstractSettingPage::resizeEvent(event);
}

void PageUserSettings::updateTable()
{
    ui->tableView->clearContent();
    int row = 0;
    for (auto iter = m_userMap.constBegin(); iter != m_userMap.constEnd(); ++iter) {
        const db_user &user = iter.value();
        if (!user.enable) {
            continue;
        }
        ui->tableView->setItemIntValue(row, ColumnID, user.id + 1);
        ui->tableView->setItemData(row, ColumnID, QVariant::fromValue(user), UserInfoRole);
        ui->tableView->setItemText(row, ColumnUserName, user.username);
        QString userType;
        switch (user.type) {
        case USERLEVEL_ADMIN:
            userType = GET_TEXT("USER/74005", "Admin");
            break;
        case USERLEVEL_OPERATOR:
            userType = GET_TEXT("USER/74006", "Operator");
            break;
        case USERLEVEL_USER:
            userType = GET_TEXT("USER/74007", "Viewer");
            break;
        case USERLEVEL_LOCAL:
            userType = GET_TEXT("USER/74054", "Local");
            break;
        }
        ui->tableView->setItemText(row, ColumnUserLevel, userType);
        if (user.type == USERLEVEL_ADMIN) {
            ui->tableView->setItemText(row, ColumnDelete, "-");
        }
        row++;
    }
}

void PageUserSettings::initializeSecurityQuestion()
{
    struct squestion sqa[3];
    memset(sqa, 0x0, sizeof(struct squestion) * 3);
    read_encrypted_list(SQLITE_FILE_NAME, sqa);

    if (sqa[0].enable == 1 && sqa[1].enable == 1 && sqa[2].enable == 1) {
        m_isQuestionSet = true;
    } else {
        m_isQuestionSet = false;
    }

    ui->comboBox_question1->setEditable(false);
    ui->comboBox_question2->setEditable(false);
    ui->comboBox_question3->setEditable(false);

    if (sqa[0].enable == 1) {
        ui->comboBox_question1->setCurrentIndex(sqa[0].sqtype);
        if (sqa[0].sqtype == SQA_CUSTOMIZED_NO) {
            ui->comboBox_question1->setCustomText(sqa[0].squestion);
        }
    } else {
        ui->comboBox_question1->setCurrentIndex(0);
    }

    if (sqa[1].enable == 1) {
        ui->comboBox_question2->setCurrentIndex(sqa[1].sqtype);
        if (sqa[1].sqtype == SQA_CUSTOMIZED_NO) {
            ui->comboBox_question2->setCustomText(sqa[1].squestion);
        }
    } else {
        ui->comboBox_question2->setCurrentIndex(0);
    }

    if (sqa[2].enable == 1) {
        ui->comboBox_question3->setCurrentIndex(sqa[2].sqtype);
        if (sqa[2].sqtype == SQA_CUSTOMIZED_NO) {
            ui->comboBox_question3->setCustomText(sqa[2].squestion);
        }
    } else {
        ui->comboBox_question3->setCurrentIndex(0);
    }
}

void PageUserSettings::onLanguageChanged()
{
    ui->tabBar->setTabText(PageUser, GET_TEXT("USER/74011", "User"));
    ui->tabBar->setTabText(PageSecurityQuestion, GET_TEXT("USER/74015", "Security Question"));

    QStringList headerList;
    headerList << "";
    headerList << GET_TEXT("CAMERASEARCH/32002", "NO.");
    headerList << GET_TEXT("COMMON/1007", "User Name");
    headerList << GET_TEXT("USER/74001", "User Level");
    headerList << GET_TEXT("COMMON/1019", "Edit");
    headerList << GET_TEXT("CHANNELMANAGE/30023", "Delete");
    ui->tableView->setHorizontalHeaderLabels(headerList);

    ui->label_adminPassword->setText(GET_TEXT("WIZARD/11007", "Admin Password "));
    ui->label_question1->setText(GET_TEXT("WIZARD/11038", "Security Question 1"));
    ui->label_question2->setText(GET_TEXT("WIZARD/11039", "Security Question 2"));
    ui->label_question3->setText(GET_TEXT("WIZARD/11040", "Security Question 3"));
    ui->label_answer1->setText(GET_TEXT("WIZARD/11041", "Security Answer 1"));
    ui->label_answer2->setText(GET_TEXT("WIZARD/11042", "Security Answer 2"));
    ui->label_answer3->setText(GET_TEXT("WIZARD/11043", "Security Answer 3"));
    ui->pushButton_question_apply->setText(GET_TEXT("COMMON/1003", "Apply"));
    ui->pushButton_question_back->setText(GET_TEXT("COMMON/1002", "Back"));

    ui->pushButton_add->setText(GET_TEXT("COMMON/1000", "Add"));
    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));
}

void PageUserSettings::onTableItemClicked(int row, int column)
{
    switch (column) {
    case ColumnEdit: {
        const db_user &user = ui->tableView->itemData(row, ColumnID, UserInfoRole).value<db_user>();
        qDebug() << gMsUser.userName() << user.username;
        if (gMsUser.isAdmin() || gMsUser.userName() == QString(user.username)) {
            EditUserDialog editUser(this);
            int result = editUser.execEdit(user);
            if (result == EditUserDialog::Accepted) {
                initializeData();
            } else if (result == EditUserDialog::TypeLogout) {
                emit sig_back();
                QTimer::singleShot(100, MainWindow::instance(), SLOT(logout()));
            }
        } else {
            ShowMessageBox(GET_TEXT("COMMON/1011", "Insufficient User Permissions."));
        }
        break;
    }
    case ColumnDelete: {
        if (!ui->tableView->itemText(row, ColumnDelete).isEmpty()) {
            break;
        }
        if (!gMsUser.isAdmin()) {
            ShowMessageBox(GET_TEXT("COMMON/1011", "Insufficient User Permissions."));
            break;
        }
        const db_user &user = ui->tableView->itemData(row, ColumnID, UserInfoRole).value<db_user>();
        int result = MessageBox::question(this, GET_TEXT("USER/74008", "Do you want to delete?"));
        if (result == MessageBox::Yes) {
            db_user tempUser;
            memset(&tempUser, 0, sizeof(db_user));
            tempUser.id = user.id;
            write_user(SQLITE_FILE_NAME, &tempUser);
            sendMessageOnly(REQUEST_FLAG_USER_UPDATE, nullptr, 0);

            initializeData();
        }
        break;
    }
    default:
        break;
    }
}

void PageUserSettings::onTabBarClicked(int index)
{
    ui->stackedWidget->setCurrentIndex(index);
}

void PageUserSettings::on_pushButton_add_clicked()
{
    AddUserDialog addUser(this);
    int result = addUser.exec();
    if (result == AddUserDialog::Accepted) {
        initializeData();
    }
}

void PageUserSettings::on_pushButton_back_clicked()
{
    emit sig_back();
}

void PageUserSettings::on_pushButton_question_apply_clicked()
{
    const db_user &adminUser = m_userMap.value(0);

    const QString &adminPassword = ui->lineEdit_adminPassword->text();
    const QString &adminPasswordMD5 = QCryptographicHash::hash(adminPassword.toLatin1(), QCryptographicHash::Md5).toHex();

    if (adminPasswordMD5 != adminUser.password) {
        ui->lineEdit_adminPassword->setCustomValid(false, GET_TEXT("MYLINETIP/112010", "Incorrect Password."));
        return;
    }
    const QString &answer1 = ui->lineEdit_answer1->text().trimmed();
    const QString &answer2 = ui->lineEdit_answer2->text().trimmed();
    const QString &answer3 = ui->lineEdit_answer3->text().trimmed();

    bool valid = ui->lineEdit_answer1->checkValid();
    valid = ui->lineEdit_answer2->checkValid() && valid;
    valid = ui->lineEdit_answer3->checkValid() && valid;
    valid = ui->comboBox_question1->checkValid() && valid;
    valid = ui->comboBox_question2->checkValid() && valid;
    valid = ui->comboBox_question3->checkValid() && valid;
    if (!valid) {
        return;
    }

    if (m_isQuestionSet) {
        int result = MessageBox::question(this, GET_TEXT("USER/74004", "Old Security Questions will be erased, please remember the new questions and answers!"));
        if (result == MessageBox::Cancel) {
            return;
        }
    }

    struct squestion sqa;
    memset(&sqa, 0x0, sizeof(struct squestion));
    sqa.enable = 1;
    sqa.sqtype = ui->comboBox_question1->currentIndex();
    snprintf(sqa.answer, sizeof(sqa.answer), "%s", answer1.toStdString().c_str());
    if (sqa.sqtype == SQA_CUSTOMIZED_NO) {
        snprintf(sqa.squestion, sizeof(sqa.squestion), "%s", ui->comboBox_question1->currentText().trimmed().toStdString().c_str());
    }
    write_encrypted_list(SQLITE_FILE_NAME, &sqa, 1);

    memset(&sqa, 0x0, sizeof(struct squestion));
    sqa.enable = 1;
    sqa.sqtype = ui->comboBox_question2->currentIndex();
    snprintf(sqa.answer, sizeof(sqa.answer), "%s", answer2.toStdString().c_str());
    if (sqa.sqtype == SQA_CUSTOMIZED_NO) {
        snprintf(sqa.squestion, sizeof(sqa.squestion), "%s", ui->comboBox_question2->currentText().trimmed().toStdString().c_str());
    }
    write_encrypted_list(SQLITE_FILE_NAME, &sqa, 2);

    memset(&sqa, 0x0, sizeof(struct squestion));
    sqa.enable = 1;
    sqa.sqtype = ui->comboBox_question3->currentIndex();
    snprintf(sqa.answer, sizeof(sqa.answer), "%s", answer3.toStdString().c_str());
    if (sqa.sqtype == SQA_CUSTOMIZED_NO) {
        snprintf(sqa.squestion, sizeof(sqa.squestion), "%s", ui->comboBox_question3->currentText().trimmed().toStdString().c_str());
    }
    write_encrypted_list(SQLITE_FILE_NAME, &sqa, 3);

    //
    if (!m_isQuestionSet) {
        ShowMessageBox(GET_TEXT("WIZARD/11059", "Security questions saved successfully!"));
    }

    ui->lineEdit_adminPassword->clear();
    ui->lineEdit_answer1->clear();
    ui->lineEdit_answer2->clear();
    ui->lineEdit_answer3->clear();
}

void PageUserSettings::on_pushButton_question_back_clicked()
{
    emit sig_back();
}

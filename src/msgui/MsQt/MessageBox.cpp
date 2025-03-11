#include "MessageBox.h"
#include "ui_MessageBox.h"
#include "AutoLogout.h"
#include "MyDebug.h"
#include "SubControl.h"
#include <QPainter>
#include <QTimer>
#include "mainwindow.h"

MessageBox *MessageBox::s_self = nullptr;

MessageBox::MessageBox(QWidget *parent)
    : BaseShadowDialog(parent)
    , ui(new Ui::MessageBox)
{
    ui->setupUi(this);

    connect(&gAutoLogout, SIGNAL(logouted()), this, SLOT(on_pushButton_cancel_clicked()));
}

MessageBox::~MessageBox()
{
    qMsDebug() << "~MessageBox()";
    delete ui;
}

void MessageBox::initialize(QWidget *parent)
{
    if (!s_self) {
        s_self = new MessageBox(parent);
    }
}

MessageBox *MessageBox::instance()
{
    return s_self;
}

void MessageBox::startTimer(int interval)
{
    m_timeout = interval;
    if (m_timer == nullptr) {
        m_timer = new QTimer(this);
        connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
        m_timer->start(1000);
    }
}

MessageBox::Result MessageBox::information(QWidget *parent, const QString &text, const QString &detail, MessageBox::Alignment alignment)
{
    MessageBox message(parent);
    message.showInformation(text, detail);

    switch (alignment) {
    case AlignNone:
        break;
    case AlignTop: {
        if (parent) {
            QRect rc = QStyle::alignedRect(Qt::LeftToRight, Qt::AlignTop, message.size(), parent->geometry());
            message.move(rc.topLeft());
        }
        break;
    }
    case AlignCenter: {
        if (parent) {
            QRect rc = QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, message.size(), parent->geometry());
            message.move(rc.topLeft());
        }
        break;
    }
    default:
        break;
    }

    return (Result)message.exec();
}

MessageBox::Result MessageBox::information(QWidget *parent, const QString &text, MessageBox::Alignment alignment)
{
    return information(parent, text, QString(), alignment);
}

/**
 * @brief MessageBox::question
 * @param parent
 * @param text
 * @param detail
 * @return Yes or Cancel
 */
MessageBox::Result MessageBox::question(QWidget *parent, const QString &text, const QString &detail, MessageBox::Alignment alignment)
{
    MessageBox message(parent);
    message.showQuestion(text, detail);
    //问题过长，改变样式
    if (message.ui->plainTextEdit->blockCount() > 4) {
        message.resize(600, 340);
        message.ui->verticalSpacer->changeSize(0, 0);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setVerticalStretch(0);
        message.ui->widget_top->setSizePolicy(sizePolicy);
        message.ui->widget_bottom->setSizePolicy(sizePolicy);
    }
    switch (alignment) {
    case AlignNone:
        break;
    case AlignTop: {
        if (parent) {
            QRect rc = QStyle::alignedRect(Qt::LeftToRight, Qt::AlignTop, message.size(), parent->geometry());
            message.move(rc.topLeft());
        }
        break;
    }
    case AlignCenter: {
        if (parent) {
            QRect rc = QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, message.size(), parent->geometry());
            message.move(rc.topLeft());
        }
        break;
    }
    default:
        break;
    }

    return (Result)message.exec();
}

MessageBox::Result MessageBox::question(QWidget *parent, const QString &text, MessageBox::Alignment alignment)
{
    return question(parent, text, QString(), alignment);
}

MessageBox::Result MessageBox::question(QWidget *parent, const QString &text, const QString &buttonText1, const QString &buttonText2)
{
    MessageBox message(parent);
    message.showQuestion(text, buttonText1, buttonText2);

    return (Result)message.exec();
}

MessageBox::Result MessageBox::warning(QWidget *parent, const QString &text, const QString &detail)
{
    MessageBox message(parent);
    message.showWarning(text, detail);
    return (Result)message.exec();
}

MessageBox::Result MessageBox::englishQuestion(QWidget *parent, const QString &text, const QString &detail)
{
    MessageBox message(parent);
    message.showEnglishQuestion(text, detail);
    return (Result)message.exec();
}

void MessageBox::message(QWidget *parent, const QString &text, const QString &detail)
{
    MessageBox message(parent);
    message.showMessage(text, detail);
    message.exec();
}

void MessageBox::queuedInformation(const QString &text, QWidget *parent)
{
    MessageBox *box = instance();
    if (parent) {
        QRect rc = QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, box->size(), QRect(parent->mapToGlobal(QPoint(0, 0)), parent->size()));
        box->move(rc.topLeft());
    } else {
        QRect screenRc = SubControl::instance()->logicalMainScreenGeometry();
        QRect rc = QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, box->size(), screenRc);
        box->move(rc.topLeft());
    }
    QMetaObject::invokeMethod(box, "showInformation", Qt::QueuedConnection, Q_ARG(QString, text));
}

MessageBox::Result MessageBox::timeoutQuestion(QWidget *parent, const QString &title, const QString &text, int seconds)
{
    MessageBox message(parent);
    message.showQuestion(text, QString());
    message.setTitle(title);
    message.startTimer(seconds);
    return (Result)message.exec();
}

bool MessageBox::isAddToVisibleList()
{
    return true;
}

void MessageBox::escapePressed()
{
    if (ui->pushButton_cancel->isVisible()) {
        on_pushButton_cancel_clicked();
    } else if (ui->pushButton_ok->isVisible()) {
        on_pushButton_ok_clicked();
    }
}

void MessageBox::returnPressed()
{
    if (ui->pushButton_ok->isVisible()) {
        on_pushButton_ok_clicked();
    }
}

void MessageBox::setText(const QString &text)
{
    ui->plainTextEdit->setPlainText(text);
}

void MessageBox::setDetail(const QString &text)
{
    int width = QFontMetrics(ui->label_detail->font()).width(text);
    if (width > 400) {
        ui->label_detail->setWordWrap(true);
    } else {
        ui->label_detail->setWordWrap(false);
    }
    ui->label_detail->setText(text);
    ui->label_detail->setVisible(!text.isEmpty());
}

void MessageBox::execInformation(const QString &text)
{
    ui->label_title->setText(GET_TEXT("LOG/64074", "Information"));
    ui->label_icon->setPixmap(QPixmap(":/common/common/infomation.png"));
    setText(text);
    setDetail("");
    ui->pushButton_cancel->hide();
    ui->pushButton_ok->show();
    ui->pushButton_yes->hide();
    onLanguageChanged();

    QRect screenRc = SubControl::instance()->logicalMainScreenGeometry();
    QRect rc = QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size(), screenRc);
    move(rc.topLeft());

    exec();
}

void MessageBox::onShowInformation(const QString &text)
{
    showInformation(text, QString());
    if (parentWidget()) {
        QRect rc = QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size(), parentWidget()->geometry());
        move(rc.topLeft());
    }
    show();
}

void MessageBox::onShowAlignCenterInformation(QWidget *widget, const QString &text)
{
    showInformation(text, QString());
    if (widget) {
        QRect rc = QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size(), widget->geometry());
        move(rc.topLeft());
    }
    show();
}

void MessageBox::onLanguageChanged()
{
    ui->pushButton_cancel->setText(GET_TEXT("COMMON/1004", "Cancel"));
    ui->pushButton_ok->setText(GET_TEXT("COMMON/1001", "OK"));
    ui->pushButton_yes->setText(GET_TEXT("PROFILE/76016", "Yes"));
}

void MessageBox::onTimeout()
{
    m_timeout--;
    ui->pushButton_cancel->setText(QString("%1(%2)").arg(GET_TEXT("COMMON/1004", "Cancel")).arg(m_timeout));

    if (m_timeout <= 0) {
        on_pushButton_cancel_clicked();
    }
}

void MessageBox::showInformation(const QString &text)
{
    ui->label_title->setText(GET_TEXT("LOG/64074", "Information"));
    ui->label_icon->setPixmap(QPixmap(":/common/common/infomation.png"));
    setText(text);
    setDetail("");
    ui->pushButton_cancel->hide();
    ui->pushButton_ok->show();
    ui->pushButton_yes->hide();
    onLanguageChanged();

    QRect screenRc = SubControl::instance()->logicalMainScreenGeometry();
    QRect rc = QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size(), screenRc);
    move(rc.topLeft());

    show();
}

void MessageBox::showQuestion(const QString &text, const QString &detail)
{
    ui->label_title->setText(GET_TEXT("LOG/64074", "Information"));
    //ui->label_icon->setPixmap(QPixmap(":/common/common/question.png"));
    ui->label_icon->setPixmap(QPixmap(":/common/common/infomation.png"));
    setText(text);
    setDetail(detail);
    ui->pushButton_cancel->show();
    ui->pushButton_ok->hide();
    ui->pushButton_yes->show();
    onLanguageChanged();
}

void MessageBox::showQuestion(const QString &text, const QString &buttonText1, const QString &buttonText2)
{
    ui->label_title->setText(GET_TEXT("LOG/64074", "Information"));
    //ui->label_icon->setPixmap(QPixmap(":/common/common/question.png"));
    ui->label_icon->setPixmap(QPixmap(":/common/common/infomation.png"));
    setText(text);
    setDetail(QString());
    ui->pushButton_cancel->show();
    ui->pushButton_ok->hide();
    ui->pushButton_yes->show();
    onLanguageChanged();

    ui->pushButton_ok->setText(buttonText1);
    ui->pushButton_cancel->setText(buttonText2);
}

void MessageBox::showWarning(const QString &text, const QString &detail)
{
    ui->label_title->setText(GET_TEXT("LOG/64074", "Information"));
    ui->label_icon->setPixmap(QPixmap(":/common/common/warning.png"));
    setText(text);
    setDetail(detail);
    ui->pushButton_cancel->hide();
    ui->pushButton_ok->show();
    ui->pushButton_yes->hide();
    onLanguageChanged();
}

void MessageBox::showEnglishQuestion(const QString &text, const QString &detail)
{
    ui->label_title->setText("Information");
    //ui->label_icon->setPixmap(QPixmap(":/common/common/question.png"));
    ui->label_icon->setPixmap(QPixmap(":/common/common/infomation.png"));
    setText(text);
    setDetail(detail);
    ui->pushButton_cancel->show();
    ui->pushButton_ok->hide();
    ui->pushButton_yes->show();
}

void MessageBox::showMessage(const QString &text, const QString &detail)
{
    ui->label_title->setText(GET_TEXT("LOG/64074", "Information"));
    ui->label_icon->setPixmap(QPixmap(":/common/common/infomation.png"));
    setText(text);
    setDetail(detail);
    ui->pushButton_cancel->hide();
    ui->pushButton_ok->hide();
    ui->pushButton_yes->hide();
    onLanguageChanged();
}

void MessageBox::on_pushButton_yes_clicked()
{
    done(Result::Yes);
    emit yesButtonClicked();
}

void MessageBox::on_pushButton_cancel_clicked()
{
    done(Result::Cancel);
    emit cancelButtonClicked();
}

void MessageBox::on_pushButton_ok_clicked()
{
    done(Result::OK);
    emit okButtonClicked();
}

void MessageBox::setTitle(const QString &text)
{
    ui->label_title->setText(text);
}

void MessageBox::showInformation(const QString &text, const QString &detail)
{
    ui->label_title->setText(GET_TEXT("LOG/64074", "Information"));
    ui->label_icon->setPixmap(QPixmap(":/common/common/infomation.png"));
    setText(text);
    setDetail(detail);
    ui->pushButton_cancel->hide();
    ui->pushButton_ok->show();
    ui->pushButton_yes->hide();
    onLanguageChanged();
}

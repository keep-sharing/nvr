#include "progressdialog.h"
#include "ui_progressdialog.h"
#include "MsLanguage.h"

ProgressDialog *ProgressDialog::s_progressDialog = nullptr;

ProgressDialog::ProgressDialog(QWidget *parent) :
    BaseShadowDialog(parent),
    ui(new Ui::ProgressDialog)
{
    ui->setupUi(this);

    s_progressDialog = this;

    connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
    onLanguageChanged();
}

ProgressDialog::~ProgressDialog()
{
    s_progressDialog = nullptr;
    delete ui;
}

ProgressDialog *ProgressDialog::instance()
{
    return s_progressDialog;
}

void ProgressDialog::setTitle(const QString &text)
{
    ui->label_title->setText(text);
}

void ProgressDialog::setMessage(const QString &text)
{
    ui->label_text->setText(text);
}

void ProgressDialog::setCancelEnable(bool enable)
{
    ui->pushButton_cancel->setEnabled(enable);
}

void ProgressDialog::showProgress()
{
    setCancelEnable(true);
    setProgressValue(0);
    show();
}

void ProgressDialog::hideProgress()
{
    hide();
    setCancelEnable(true);
}

void ProgressDialog::setProgressValue(int value)
{
    ui->progressBar->setValue(value);
}

void ProgressDialog::onLanguageChanged()
{
    ui->label_title->setText(GET_TEXT("LOG/64074", "Information"));
    ui->label_text->setText(GET_TEXT("COMMON/1032", "Please wait..."));
    ui->pushButton_cancel->setText(GET_TEXT("COMMON/1004", "Cancel"));
}

void ProgressDialog::on_pushButton_cancel_clicked()
{
    ui->pushButton_cancel->clearUnderMouse();
    ui->pushButton_cancel->clearFocus();
    setCancelEnable(false);
    emit sig_cancel();
}

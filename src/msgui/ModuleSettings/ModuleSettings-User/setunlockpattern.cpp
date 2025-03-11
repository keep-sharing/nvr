#include "setunlockpattern.h"
#include "ui_setunlockpattern.h"
#include "MyDebug.h"
#include "MsLanguage.h"

SetUnlockPattern *SetUnlockPattern::s_setUnlockPattern = nullptr;

SetUnlockPattern::SetUnlockPattern(QWidget *parent) :
    BaseShadowDialog(parent),
    ui(new Ui::SetUnlockPattern)
{
    s_setUnlockPattern = this;
    ui->setupUi(this);
    ui->widget_pattern->setEnabled(true);
    ui->widget_pattern->setStyle(GrayStyle);
    ui->pushButton_ok->setEnabled(false);
    ui->label_patternTips->setText(GET_TEXT("USER/74063", "Please connect at least 4 dots."));
    ui->label_title->setText(GET_TEXT("USER/74062", "Set Unlock Pattern"));
    ui->pushButton_ok->setText(GET_TEXT("COMMON/1001", "OK"));
    ui->pushButton_cancel->setText(GET_TEXT("COMMON/1004", "Cancel"));

    connect(ui->widget_pattern, SIGNAL(drawStart()), this, SLOT(onDrawStart()));
    connect(ui->widget_pattern, SIGNAL(drawFinished(QString)), this, SLOT(onDrawFinished(QString)));
    m_pattern_text = "0";
    step = 1;
}

SetUnlockPattern::~SetUnlockPattern()
{
    s_setUnlockPattern = nullptr;
    delete ui;
}

SetUnlockPattern *SetUnlockPattern::instance()
{
    return s_setUnlockPattern;
}

QString SetUnlockPattern::getText() const
{
    return m_pattern_text;
}

void SetUnlockPattern::onDrawStart()
{
    ui->pushButton_ok->setEnabled(false);
    ui->label_patternTips->setText(GET_TEXT("USER/74060", "Release mouse when finished."));
}

void SetUnlockPattern::onDrawFinished(QString text)
{
    qMsDebug() << QString("onDrawFinished!!!  m_pattern_text:[%1] : [%2]").arg(m_pattern_text).arg(text);

    if(step == 1)
    {
        if(text.size() < 4)
        {
            ui->label_patternTips->setText(GET_TEXT("USER/74056", "Connect at least 4 dots, please try again."));
            return;
        }

        step++;
        m_pattern_text = text;
        ui->label_patternTips->setText(GET_TEXT("USER/74057", "Please draw again to confirm."));
        return;
    }
    else
    {
        step--;
        if(m_pattern_text == text)
        {
            ui->pushButton_ok->setEnabled(true);
            ui->label_patternTips->setText(GET_TEXT("USER/74058", "Set unlock pattern successfully."));
        }
        else
        {
            m_pattern_text = "0";
            ui->label_patternTips->setText(GET_TEXT("USER/74059", "Unmatched pattern, please try again."));
        }
    }
}

void SetUnlockPattern::on_pushButton_ok_clicked()
{
    accept();
}

void SetUnlockPattern::on_pushButton_cancel_clicked()
{
    step = 1;
    m_pattern_text = "0";
    reject();
}

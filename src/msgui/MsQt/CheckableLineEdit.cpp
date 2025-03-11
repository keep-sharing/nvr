#include "CheckableLineEdit.h"
#include "ui_CheckableLineEdit.h"
#include "MyDebug.h"
#include "MyLineEditTip.h"
#include "MsLanguage.h"
#include <QPainter>
#include <QStyle>
#include <QTimer>

const int TipHeight = 26;

CheckableLineEdit::CheckableLineEdit(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CheckableLineEdit)
{
    ui->setupUi(this);

    ui->lineEdit->setContextMenuPolicy(Qt::NoContextMenu);

    QRegExp rx(R"(\d*)");
    QRegExpValidator *validator = new QRegExpValidator(rx, this);
    ui->lineEdit->setValidator(validator);

    connect(ui->checkBox, SIGNAL(checkStateSet(int)), this, SLOT(onCheckBoxStateSet(int)));

    //
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    m_timer->setInterval(5000);
}

CheckableLineEdit::~CheckableLineEdit()
{
    delete ui;
}

void CheckableLineEdit::setRange(int min, int max)
{
    m_minValue = min;
    m_maxValue = max;

    QString str = QString::number(m_maxValue);
    ui->lineEdit->setMaxLength(str.size());
}

void CheckableLineEdit::setChecked(bool checked)
{
    ui->checkBox->setChecked(checked);
    update();
}

bool CheckableLineEdit::isChecked() const
{
    return ui->checkBox->isChecked();
}

void CheckableLineEdit::setValue(int value)
{
    ui->lineEdit->clear();
    if (value >= m_minValue && value <= m_maxValue) {
        ui->lineEdit->setText(QString::number(value));
    }
}

int CheckableLineEdit::value() const
{
    bool ok = false;
    int value = ui->lineEdit->text().toInt(&ok);
    if (!ok || value < m_minValue || value > m_maxValue) {
        return -1;
    } else {
        return value;
    }
}

void CheckableLineEdit::setEnabled(bool enable)
{
    ui->checkBox->reset();
    QWidget::setEnabled(enable);
}

bool CheckableLineEdit::isValid() const
{
    return m_valid;
}

void CheckableLineEdit::setValid(bool newValid)
{
    if (m_valid == newValid) {
        if (!m_valid) {
            showWarningTip();
        }
        return;
    }
    m_valid = newValid;
    style()->polish(this);
    update();

    if (m_valid) {
        hideWarningTip();
    } else {
        showWarningTip();
    }

    emit validChanged();
}

void CheckableLineEdit::clearCheck()
{
    m_valid = true;
    style()->polish(this);
    hideWarningTip();
}

bool CheckableLineEdit::checkValid()
{
    if (!isVisible()) {
        return true;
    }
    onEditFinished();
    return isValid();
}

void CheckableLineEdit::setTipString(const QString &str)
{
    if (!m_invalidTip) {
        m_invalidTip = new MyLineEditTip(this);
    }
    m_invalidTip->setText(str);
}

void CheckableLineEdit::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
}

void CheckableLineEdit::hideEvent(QHideEvent *event)
{
    setValid(true);
    QWidget::hideEvent(event);
}

void CheckableLineEdit::changeEvent(QEvent * event)
{
    if (event->type() == QEvent::EnabledChange) {
        if (!isEnabled()) {
            if (!m_invalidTip) {
                m_invalidTip = new MyLineEditTip(this);
            }
            m_invalidTip->setEnabled(true);
            m_invalidTip->update();
        }
    }
}

void CheckableLineEdit::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    QRect rc = rect();
    rc.adjust(0, 0, -1, -1);

    if (isValid()) {
        painter.setPen(QColor(185, 185, 185));
        if (ui->checkBox->isChecked() && isEnabled()) {
            painter.setBrush(QColor(225, 225, 225));
        } else {
            painter.setBrush(QColor(190, 190, 190));
        }
    } else {
        painter.setPen(QColor(231, 94, 93));
        painter.setBrush(QColor(234, 220, 217));
    }
    painter.drawRect(rc);
}

void CheckableLineEdit::showWarningTip()
{
    if (!isVisible()) {
        return;
    }

    if (!m_invalidTip) {
        m_invalidTip = new MyLineEditTip(this);
    }
    QPoint p = mapToGlobal(QPoint(0, 0));
    QRect rc(p.x(), p.y() + height() - 1, width(), TipHeight);
    m_invalidTip->setGeometry(rc);
    m_invalidTip->show();
    m_timer->start();
}

void CheckableLineEdit::hideWarningTip()
{
    if (m_invalidTip) {
        m_invalidTip->hide();
    }
}

void CheckableLineEdit::onEditFinished()
{
    const QString &str = ui->lineEdit->text();
    do {
        bool ok;
        int value = str.toInt(&ok);
        if (!ok || value < m_minValue || value > m_maxValue) {
            setTipString(GET_TEXT("MYLINETIP/112003", "Valid range: %1-%2.").arg(m_minValue).arg(m_maxValue));
            break;
        }

        //valid
        setValid(true);
        return;
    } while (0);
    //invalid
    setValid(false);
}

void CheckableLineEdit::onCheckBoxStateSet(int state)
{
    bool checked = (state == Qt::Checked);
    ui->lineEdit->setEnabled(checked);

    update();
}

void CheckableLineEdit::onTimeout()
{
    setValid(true);
}

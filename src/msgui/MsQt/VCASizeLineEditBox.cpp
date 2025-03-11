#include "VCASizeLineEditBox.h"
#include "ui_VCASizeLineEditBox.h"
#include "MyDebug.h"
#include "MyLineEditTip.h"
#include "MsLanguage.h"
#include <QPainter>
#include <QStyle>
#include <QTimer>

const int TipHeight = 26;

VCASizeLineEditBox::VCASizeLineEditBox(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VCASizeLineEditBox)
{
    ui->setupUi(this);
    setContextMenuPolicy(Qt::NoContextMenu);
    ui->lineEditWidth->setContextMenuPolicy(Qt::NoContextMenu);
    ui->lineEditHeight->setContextMenuPolicy(Qt::NoContextMenu);

    //
    QRegExp rx(R"(\d*)");
    QRegExpValidator *validator = new QRegExpValidator(rx, this);
    ui->lineEditWidth->setValidator(validator);
    ui->lineEditHeight->setValidator(validator);

    //
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    m_timer->setInterval(5000);
}

VCASizeLineEditBox::~VCASizeLineEditBox()
{
    delete ui;
}

void VCASizeLineEditBox::setHeightRange(int min, int max)
{
    m_minHeightValue = min;
    m_maxHeightValue =max;

    QString str = QString::number(m_maxHeightValue);
    ui->lineEditHeight->setMaxLength(str.size());
}

void VCASizeLineEditBox::setWidthRange(int min, int max)
{
    m_minWidthValue = min;
    m_maxWidthValue = max;

    QString str = QString::number(m_maxWidthValue);
    ui->lineEditWidth->setMaxLength(str.size());
}

void VCASizeLineEditBox::setWidthValue(int width)
{
    ui->lineEditWidth->clear();
    ui->lineEditWidth->setText(QString::number(width));
    m_widthValue = width;
}

int VCASizeLineEditBox::widthValue()
{
    if (ui->lineEditWidth->text().isEmpty()) {
        return 0;
    }
    return m_widthValue;
}

void VCASizeLineEditBox::setHeightValue(int height)
{
    ui->lineEditHeight->clear();
    ui->lineEditHeight->setText(QString::number(height));
    m_heightValue = height;
}

int VCASizeLineEditBox::heightValue()
{
    if (ui->lineEditHeight->text().isEmpty()) {
        return 0;
    }
    return m_heightValue;
}

void VCASizeLineEditBox::setEnabled(bool enable)
{
    QWidget::setEnabled(enable);
}

bool VCASizeLineEditBox::isValid() const
{
    return m_valid;
}

void VCASizeLineEditBox::setValid(bool newValid)
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

bool VCASizeLineEditBox::checkValid()
{
    if (!isVisible() ) {
        return true;
    }
    onEditFinished();
    return isValid();
}

void VCASizeLineEditBox::setTipString(const QString &str)
{
    if (!m_invalidTip) {
        m_invalidTip = new MyLineEditTip(this);
    }
    m_invalidTip->setText(str);
}

void VCASizeLineEditBox::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
}

void VCASizeLineEditBox::hideEvent(QHideEvent *event)
{
    setValid(true);
    QWidget::hideEvent(event);
}

void VCASizeLineEditBox::changeEvent(QEvent *event)
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
    QWidget::changeEvent(event);
}

void VCASizeLineEditBox::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    QRect rc = rect();
    rc.adjust(0, 0, -1, -1);

    if (isValid()) {
        painter.setPen(QColor(0, 0, 0, 0));
        painter.setBrush(QColor(255, 255, 255));
    } else {
        painter.setPen(QColor(231, 94, 93));
        painter.setBrush(QColor(234, 220, 217));
    }
    painter.drawRect(rc);
}

void VCASizeLineEditBox::showWarningTip()
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

void VCASizeLineEditBox::hideWarningTip()
{
    if (m_invalidTip) {
        m_invalidTip->hide();
    }
}

void VCASizeLineEditBox::onTimeout()
{
    setValid(true);
}

void VCASizeLineEditBox::onEditFinished()
{
    const QString &str = ui->lineEditHeight->text();
    const QString &str2 = ui->lineEditWidth->text();
    do {
        bool ok;
        int value = str.toInt(&ok);
        int value2 = str2.toInt(&ok);
        if (!ok || value < m_minHeightValue || value > m_maxHeightValue || value2 < m_minWidthValue || value2 > m_maxWidthValue
                || str.isEmpty() || str2.isEmpty()) {
            setTipString(GET_TEXT("STATUS/177012", "Valid range:%1x%2-%3x%4.").arg(m_minWidthValue).arg(m_minHeightValue).arg(m_maxWidthValue).arg(m_maxHeightValue));
            break;
        }
        //valid
        setValid(true);
        return;
    } while (0);
    //invalid
    setValid(false);
}

void VCASizeLineEditBox::on_lineEditWidth_editingFinished()
{
    m_widthValue = ui->lineEditWidth->text().toInt();
    emit widthChange();
}

void VCASizeLineEditBox::on_lineEditHeight_editingFinished()
{
    m_heightValue = ui->lineEditHeight->text().toInt();
    emit heightChange();
}

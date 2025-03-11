#include "mylineedit.h"
#include "MyDebug.h"
#include "MyLineEditTip.h"
#include "MsLanguage.h"
#include "myqt.h"
#include <QEvent>
#include <QStyle>
#include <QToolButton>
#include <QLabel>
#include <QTimer>

const int TipHeight = 26;

MyLineEdit::MyLineEdit(QWidget *parent)
    : QLineEdit(parent)
{
    setContextMenuPolicy(Qt::NoContextMenu);

    //connect(this, SIGNAL(editingFinished()), this, SLOT(onEditFinished()));

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    m_timer->setInterval(5000);
}

MyLineEdit::~MyLineEdit()
{
}

MyLineEdit::CheckMode MyLineEdit::checkMode() const
{
    return m_checkMode;
}

void MyLineEdit::setCheckMode(CheckMode newCheckMode)
{
    m_checkMode = newCheckMode;
}

void MyLineEdit::setCheckMode(CheckMode newCheckMode, int min, int max)
{
    setCheckMode(newCheckMode, min, max, 0);
}

void MyLineEdit::setCheckMode(CheckMode newCheckMode, int min, int max, int val)
{
    m_checkMode = newCheckMode;
    m_minValue = min;
    m_maxValue = max;
    m_value = val;
    QRegExp rx(R"(\d*)");
    QRegExpValidator *validator = new QRegExpValidator(rx, this);
    setValidator(validator);

    switch (newCheckMode) {
    case RangeCheck:
    case RangeCanEmptyCheck:
        setMaxLength(QString::number(max).size());
        break;
    default:
        break;
    }
}

bool MyLineEdit::checkValid()
{
    if (!isVisible()) {
        return true;
    }
    onEditFinished();
    return isValid();
}

void MyLineEdit::setCustomValid(bool valid, const QString &tip)
{
    setTipString(tip);
    setValid(valid);
}

bool MyLineEdit::isValid() const
{
    return m_valid;
}

void MyLineEdit::setValid(bool newValid)
{
    if (m_valid == newValid) {
        if (!m_valid) {
            showWarningTip();
        }
        return;
    }
    m_valid = newValid;
    style()->polish(this);

    if (m_valid) {
        hideWarningTip();
    } else {
        showWarningTip();
    }

    emit validChanged();
}

void MyLineEdit::setTipString(const QString &str)
{
    if (!m_invalidTip) {
        m_invalidTip = new MyLineEditTip(this);
    }
    m_invalidTip->setText(str);
}

void MyLineEdit::setTipFronSize(const int size)
{
    if (!m_invalidTip) {
        m_invalidTip = new MyLineEditTip(this);
    }
    m_invalidTip->setTextSize(size);
}

RtspInfo MyLineEdit::rtspInfo() const
{
    return m_rtspInfo;
}

bool MyLineEdit::checkMail(const char *pszEmail)
{
    if (pszEmail == NULL) {
        return false;
    }
    int iAtPos = 0;
    int iLastDotPos = 0;
    int i = 0;
    int iAtTimes = 0;
    while (*(pszEmail + i) != '\0') {
        char ch = *(pszEmail + i);
        if (!isprint(ch) || isspace(ch)) //空格和控制字符是非法的，限制得还比较宽松
        {
            iAtTimes = 0;
            break;
        }
        if (ch == '@') {
            iAtPos = i;
            iAtTimes++;
        } else if (ch == '.') {
            iLastDotPos = i;
        }
        i++;
    }
    if (i > 64 || iAtPos < 1 || (iLastDotPos - 2) < iAtPos || (i - iLastDotPos) < 3 || (i - iLastDotPos) > 5 || iAtTimes > 1 || iAtTimes == 0) //对@以及域名依靠位置来判断，限制长度为64
    {
        return false;
    }
    return true;
}

bool MyLineEdit::checkSpecialStr(const QString &str)
{
    QRegExp rx("[^&\'\"\\\\]*");
    if (!rx.exactMatch(str)) {
        return false;
    }
    return true;
}

bool MyLineEdit::checkMACAddress(const QString &str)
{
    QStringList strList = str.split(":");
    if (strList.count() != 6) {
        return false;
    }
    QString pstr;
    for (int i = 0; i < 6; i++) {
        pstr = strList.at(i);
        QRegExp rx("[0-9a-fA-F]{2}");
        if (pstr.length() != 2 || rx.indexIn(pstr) == -1) {
            return false;
        }
    }
    return true;
}

bool MyLineEdit::checkSmnp(const QString &str)
{
    QRegExp forbidRx(QString("[^ ~!#()`;&/\\\\:\\*\?\'\"<>|%]*"));
    if (!forbidRx.exactMatch(str)) {
        return false;
    }
    return true;
}

bool MyLineEdit::checkTooLong()
{
    const QString &str = text();
    if (str.isEmpty() || str.length() < 8 || str.length() > 32) {
        return false;
    }
    QRegExp pwdRx(QString("[^0-9]*"));
    QRegExp passwordRx(QString("[^A-Za-z]*"));
    if (pwdRx.exactMatch(str) || passwordRx.exactMatch(str)) {
        return true;
    }
    return false;
}

void MyLineEdit::showWarningTip()
{
    if (!isEnabled() && AudioCheck != m_checkMode  && PTZRangeCheck != m_checkMode) {
        return;
    }
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

void MyLineEdit::hideWarningTip()
{
    if (m_invalidTip) {
        m_invalidTip->hide();
    }
}

void MyLineEdit::hideEvent(QHideEvent *event)
{
    setValid(true);
    QLineEdit::hideEvent(event);
}

void MyLineEdit::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::EnabledChange) {
        if (isEnabled()) {
            if (!isValid()) {
                QMetaObject::invokeMethod(this, "showWarningTip", Qt::QueuedConnection);
            }
        } else {
            QMetaObject::invokeMethod(this, "hideWarningTip", Qt::QueuedConnection);
            if (!m_invalidTip) {
                m_invalidTip = new MyLineEditTip(this);
            }
            m_invalidTip->setEnabled(true);
            m_invalidTip->update();
        }
    }
    QLineEdit::changeEvent(event);
}

void MyLineEdit::moveEvent(QMoveEvent * event)
{
    Q_UNUSED(event);
    // refresh cordinate
    if (!m_valid) {
        QPoint p = mapToGlobal(QPoint(0, 0));
        QRect rc(p.x(), p.y() + height() - 1, width(), TipHeight);
        m_invalidTip->setGeometry(rc);
        m_invalidTip->show();
    }
}

void MyLineEdit::onEditFinished()
{
    if (m_checkMode == NoCheck) {
        return;
    }

    const QString &str = text();

    do {
        if (IPCheck == m_checkMode) {
            if (str.isEmpty()) {
                setTipString(GET_TEXT("MYLINETIP/112000", "Cannot be empty."));
                break;
            }
            if (!MyQt::isValidIP(str) && !MyQt::isValidIPv6(str)) {
                setTipString(GET_TEXT("MYLINETIP/112001", "Invalid."));
                break;
            }
        } else if (IPv4Check == m_checkMode) {
            if (str.isEmpty()) {
                setTipString(GET_TEXT("MYLINETIP/112000", "Cannot be empty."));
                break;
            }
            if (!MyQt::isValidIP(str)) {
                setTipString(GET_TEXT("MYLINETIP/112001", "Invalid."));
                break;
            }
        } else if (IPv6Check == m_checkMode) {
            if (str.isEmpty()) {
                setTipString(GET_TEXT("MYLINETIP/112000", "Cannot be empty."));
                break;
            }
            if (!MyQt::isValidIPv6(str)) {
                setTipString(GET_TEXT("MYLINETIP/112001", "Invalid."));
                break;
            }
        } else if (RtspCheck == m_checkMode) {
            if (str.isEmpty()) {
                setTipString(GET_TEXT("MYLINETIP/112000", "Cannot be empty."));
                break;
            }
            m_rtspInfo.setRtsp(str);
            if (!m_rtspInfo.isValid()) {
                setTipString(GET_TEXT("MYLINETIP/112002", "Invalid RTSP url."));
                break;
            }
        } else if (RangeCheck == m_checkMode) {
            bool ok;
            int value = text().toInt(&ok);
            if (!ok || value < m_minValue || value > m_maxValue) {
                setTipString(GET_TEXT("MYLINETIP/112003", "Valid range: %1-%2.").arg(m_minValue).arg(m_maxValue));
                break;
            }
        } else if (RangeCanEmptyCheck == m_checkMode) {
            bool ok;
            int value = text().toInt(&ok);
            if (!str.isEmpty() && (!ok || value < m_minValue || value > m_maxValue) ) {
                setTipString(GET_TEXT("MYLINETIP/112020", "Valid range: empty or %1-%2.").arg(m_minValue).arg(m_maxValue));
                break;
            }
        } else if (SpecialRangeCheck == m_checkMode) {
            bool ok;
            int value = text().toInt(&ok);
            if (!ok || ((value < m_minValue || value > m_maxValue) && value != m_value)) {
                setTipString(GET_TEXT("MYLINETIP/112009", "Valid range: %1 or %2-%3.").arg(m_value).arg(m_minValue).arg(m_maxValue));
                break;
            }
        } else if (UserNameCheck == m_checkMode) {
            if (str.isEmpty()) {
                setTipString(GET_TEXT("MYLINETIP/112000", "Cannot be empty."));
                break;
            }
            QRegExp rx(R"([&/\\:\*\?'"<>\|%])");
            if (rx.indexIn(str) != -1) {
                setTipString(GET_TEXT("MYLINETIP/112004", "Invalid characters: &/\\:*?'\"<>|%."));
                break;
            }
        } else if (EmptyCheck == m_checkMode) {
            if (str.isEmpty()) {
                setTipString(GET_TEXT("MYLINETIP/112000", "Cannot be empty."));
                break;
            }
        } else if (PasswordCheck == m_checkMode) {
            if (str.isEmpty() || str.length() < 8 || str.length() > 32) {
                setTipString(GET_TEXT("MYLINETIP/112006", "Must be 8 to 32 characters long."));
                break;
            }
            QRegExp pwdRx(QString("[^0-9]*"));
            QRegExp passwordRx(QString("[^A-Za-z]*"));
            if (pwdRx.exactMatch(str) || passwordRx.exactMatch(str)) {
                setTipString(GET_TEXT("MYLINETIP/112007", "Must contain at least one number and one letter."));
                break;
            }
        } else if (IPv4CanEmptyCheck == m_checkMode) {
            if (!MyQt::isValidIP(str) && !str.isEmpty()) {
                setTipString(GET_TEXT("MYLINETIP/112001", "Invalid."));
                break;
            }
        } else if (IPv6CanEmptyCheck == m_checkMode) {
            if (!MyQt::isValidIPv6(str) && !str.isEmpty()) {
                setTipString(GET_TEXT("MYLINETIP/112001", "Invalid."));
                break;
            }
        } else if (SpecialStrCheck == m_checkMode) {
            if (!checkSpecialStr(str)) {
                setTipString(GET_TEXT("MYLINETIP/112014", "Invalid characters: &\"\\."));
                break;
            }
        } else if (DDNSCheck == m_checkMode) {
            if (str.isEmpty()) {
                setTipString(GET_TEXT("MYLINETIP/112000", "Cannot be empty."));
                break;
            }
            if (!checkSpecialStr(str)) {
                setTipString(GET_TEXT("MYLINETIP/112014", "Invalid characters: &\"\\.'"));
                break;
            }
        } else if (EmailCheck == m_checkMode) {
            if (str.isEmpty()) {
                setTipString(GET_TEXT("MYLINETIP/112000", "Cannot be empty."));
                break;
            }
            if (!checkMail(str.trimmed().toStdString().c_str())) {
                setTipString(GET_TEXT("MYLINETIP/112001", "Invalid."));
                break;
            }
            if (!checkSpecialStr(str)) {
                setTipString(GET_TEXT("MYLINETIP/112014", "Invalid characters: &\"\\."));
                break;
            }
        } else if (EmailCanEmptyCheck == m_checkMode) {
            if (!checkMail(str.trimmed().toStdString().c_str()) && !str.isEmpty()) {
                setTipString(GET_TEXT("MYLINETIP/112001", "Invalid."));
                break;
            }
            if (!checkSpecialStr(str)) {
                setTipString(GET_TEXT("MYLINETIP/112014", "Invalid characters: &\"\\."));
                break;
            }
        } else if (MACCheck == m_checkMode) {
            if (str.isEmpty()) {
                setTipString(GET_TEXT("MYLINETIP/112000", "Cannot be empty."));
                break;
            }
            if (!checkMACAddress(str)) {
                setTipString(GET_TEXT("MYLINETIP/112001", "Invalid."));
                break;
            }
        } else if (ServerCheck == m_checkMode) {
            if (str.isEmpty()) {
                setTipString(GET_TEXT("MYLINETIP/112000", "Cannot be empty."));
                break;
            }
            if (!MyQt::isValidAddress(str)) {
                setTipString(GET_TEXT("MYLINETIP/112001", "Invalid."));
                break;
            }
        } else if (SnmpCheck == m_checkMode) {
            if (isEnabled() && str.isEmpty()) {
                setTipString(GET_TEXT("MYLINETIP/112000", "Cannot be empty."));
                break;
            }
            if (isEnabled() && !checkSmnp(str)) {
                setTipString(GET_TEXT("MYLINETIP/112015", "Invalid characters: ~!#();&/:*?'\"<>|%Space."));
                break;
            }
        } else if (SnmpPasswordCheck == m_checkMode) {
            if (str.isEmpty() || str.length() < 8 || str.length() > 32) {
                setTipString(GET_TEXT("MYLINETIP/112006", "Must be 8 to 32 characters long."));
                break;
            }
            if (!checkSmnp(str)) {
                setTipString(GET_TEXT("MYLINETIP/112015", "Invalid characters: ~!#();&/:*?'\"<>|%Space."));
                break;
            }
        } else if (FolderNameCheck == m_checkMode) {
            if (str.trimmed().isEmpty()) {
                setTipString(GET_TEXT("MYLINETIP/112000", "Cannot be empty."));
                break;
            }
            static QRegExp rx(R"([\\/:\*\?"<>\|])");
            if (str.contains(rx)) {
                setTipString(GET_TEXT("MYLINETIP/112016", R"(Invalid characters:\/:*?"<>|.)"));
                break;
            }
            if (str.at(0) == QChar('.') || str.endsWith(".")) {
                setTipString(GET_TEXT("MYLINETIP/112017", "Incorrect position of character .."));
                break;
            }
        } else if (NasDirectoryCheck == m_checkMode) {
            if (str.isEmpty()) {
                setTipString(GET_TEXT("MYLINETIP/112000", "Cannot be empty."));
                break;
            }
            static QRegExp rx(R"([\\:\*\?"<>\|])");
            if (str.contains(rx)) {
                setTipString(GET_TEXT("MYLINETIP/112019", R"(Invalid characters:\:*?"<>|.)"));
                break;
            }
        } else if (WizardNetCheck == m_checkMode) {
            if (str.isEmpty()) {
                setTipString(GET_TEXT("MYLINETIP/112000", "Cannot be empty."));
                break;
            }
            QRegExp rx("^(\\d{1,2}|1\\d\\d|2[0-4]\\d|25[0-5])[.](\\d{1,2}|1\\d\\d|2[0-4]\\d|25[0-5])[.](\\d{1,2}|1\\d\\d|2[0-4]\\d|25[0-5])[.](\\d{1,2}|1\\d\\d|2[0-4]\\d|25[0-5])$");
            if (!rx.exactMatch(str)) {
                setTipString(GET_TEXT("MYLINETIP/112001", "Invalid."));
                break;
            }
        } else if (WizardNetCanEmptyCheck == m_checkMode) {
            QRegExp rx("^(\\d{1,2}|1\\d\\d|2[0-4]\\d|25[0-5])[.](\\d{1,2}|1\\d\\d|2[0-4]\\d|25[0-5])[.](\\d{1,2}|1\\d\\d|2[0-4]\\d|25[0-5])[.](\\d{1,2}|1\\d\\d|2[0-4]\\d|25[0-5])$");
            if (!rx.exactMatch(str) && !str.isEmpty()) {
                setTipString(GET_TEXT("MYLINETIP/112001", "Invalid."));
                break;
            }
        } else if (AudioCheck == m_checkMode) {
            if (str.isEmpty()) {
                setTipString(GET_TEXT("MYLINETIP/112000", "Cannot be empty."));
                break;
            }
        } else if (PTZRangeCheck == m_checkMode) {
            bool ok;
            int value = text().toInt(&ok);
            if (!ok || value < m_minValue || value > m_maxValue) {
                setTipString(GET_TEXT("MYLINETIP/112003", "Valid range: %1-%2.").arg(m_minValue).arg(m_maxValue));
                break;
            }
        }else {
            qMsWarning() << "invalid mode:" << m_checkMode;
        }
        //valid
        setValid(true);
        return;
    } while (0);
    //invalid
    setValid(false);
}

void MyLineEdit::onTimeout()
{
    setValid(true);
}

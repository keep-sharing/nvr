#include "EditAccessFilter.h"
#include "ui_EditAccessFilter.h"
#include "MyDebug.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include "myqt.h"

EditAccessFilter *EditAccessFilter::s_EditAccessFilter = nullptr;

EditAccessFilter::EditAccessFilter(QWidget *parent)
    : BaseShadowDialog(parent)
    , ui(new Ui::EditAccessFilter)
{
    s_EditAccessFilter = this;
    ui->setupUi(this);

    ui->comboBox_addressType->clear();
    ui->comboBox_addressType->addItem(GET_TEXT("DEVICEINFO/60006", "MAC Address"), 0);
    ui->comboBox_addressType->addItem(GET_TEXT("COMMON/1033", "IP Address"), 1);
    ui->comboBox_ipRule->clear();
    ui->comboBox_ipRule->addItem(GET_TEXT("ACCESSFILTER/81006", "Single"), 0);
    ui->comboBox_ipRule->addItem(GET_TEXT("ACCESSFILTER/81007", "Range"), 1);

    ui->lineEdit_ipSingle->setCheckMode(MyLineEdit::IPv4Check);
    ui->lineEdit_ipRangeStart->setCheckMode(MyLineEdit::IPv4Check);
    ui->lineEdit_ipRangeEnd->setCheckMode(MyLineEdit::IPv4Check);
    ui->lineEdit_mac->setCheckMode(MyLineEdit::MACCheck);

    QRegExp regx("[a-zA-Z0-9:]+$");
    QValidator *validator = new QRegExpValidator(regx, this );
    ui->lineEdit_mac->setValidator( validator );
    onLanguageChanged();
}

EditAccessFilter::~EditAccessFilter()
{
    s_EditAccessFilter = nullptr;
    delete ui;
}

EditAccessFilter *EditAccessFilter::instance()
{
    return s_EditAccessFilter;
}

QString EditAccessFilter::getAddress() const
{
    return m_address;
}

void EditAccessFilter::on_comboBox_addressType_activated(int index)
{
    ui->comboBox_addressType->setCurrentIndex(index);
    if (index) { //ip
        ui->label_mac->setVisible(false);
        ui->lineEdit_mac->setVisible(false);

        ui->label_ipRule->setVisible(true);
        ui->comboBox_ipRule->setVisible(true);
        on_comboBox_ipRule_activated(0);
    } else { //mac
        ui->label_mac->setVisible(true);
        ui->lineEdit_mac->setVisible(true);

        ui->label_ipRule->setVisible(false);
        ui->comboBox_ipRule->setVisible(false);

        ui->label_ipSingle->setVisible(false);
        ui->lineEdit_ipSingle->setVisible(false);

        ui->label_ipRange->setVisible(false);
        ui->lineEdit_ipRangeStart->setVisible(false);
        ui->label_->setVisible(false);
        ui->lineEdit_ipRangeEnd->setVisible(false);

        if (m_address.contains(":")) {
            ui->lineEdit_mac->setText(m_address);
        }
    }
}

void EditAccessFilter::on_comboBox_ipRule_activated(int index)
{ //0:single;  1:range
    ui->comboBox_ipRule->setCurrentIndex(index);
    bool visible = (index == 0);

    ui->label_ipSingle->setVisible(visible);
    ui->lineEdit_ipSingle->setVisible(visible);

    ui->label_ipRange->setVisible(!visible);
    ui->lineEdit_ipRangeStart->setVisible(!visible);
    ui->label_->setVisible(!visible);
    ui->lineEdit_ipRangeEnd->setVisible(!visible);

    if (m_address.contains("-")) {
        QStringList list = m_address.split("-");
        ui->lineEdit_ipRangeStart->setText(list[0]);
        ui->lineEdit_ipRangeEnd->setText(list[1]);
    } else if (m_address.contains(".")) {
        ui->lineEdit_ipSingle->setText(m_address);
    }
}

void EditAccessFilter::initializeData(int index, QList<access_list> filterList)
{
    m_index = index;
    m_filterList = filterList;
    int type = ADDRESS_TYPE_IP_SINGLE;

    if (m_index >= m_filterList.length()) { //add address
        m_address = "0";
        ui->label_title->setText(GET_TEXT("ACCESSFILTER/81004", "Address Add"));
        on_comboBox_addressType_activated(1);
    } else { //edit address
        type = m_filterList.at(m_index).type;
        m_address = m_filterList.at(m_index).address;
        ui->label_title->setText(GET_TEXT("ACCESSFILTER/81005", "Address Edit"));
        if (type == ADDRESS_TYPE_MAC) {
            on_comboBox_addressType_activated(0);
        } else if (type == ADDRESS_TYPE_IP_RANGE) {
            on_comboBox_addressType_activated(1);
            on_comboBox_ipRule_activated(1);
        } else if (type == ADDRESS_TYPE_IP_SINGLE) {
            on_comboBox_addressType_activated(1);
            on_comboBox_ipRule_activated(0);
        } else {
            on_pushButton_cancel_clicked();
        }
    }
    qMsDebug() << QString("m_index:[%1], length:[%2],m_address:[%3]").arg(m_index).arg(m_filterList.length()).arg(m_address);

    onLanguageChanged();
}

void EditAccessFilter::onLanguageChanged()
{
    ui->label_addressType->setText(GET_TEXT("ACCESSFILTER/81010", "Address Type"));
    ui->label_mac->setText(GET_TEXT("DEVICEINFO/60006", "MAC Address"));
    ui->label_ipRule->setText(GET_TEXT("ACCESSFILTER/81011", "IP Address Rule"));
    ui->label_ipSingle->setText(GET_TEXT("COMMON/1033", "IP Address"));
    ui->label_ipRange->setText(GET_TEXT("COMMON/1033", "IP Address"));
}

QString EditAccessFilter::ipAdjust(QString str)
{
    int i, val;
    QString pStr;
    QString ret = "";
    QStringList strList;
    strList = str.split(".");
    for (i = 0; i < 4; i++) {
        pStr = strList[i];
        val = pStr.toInt();
        ret += QString("%1").arg(val);
        if (i < 3)
            ret += QString(".");
    }
    return ret;
}

int EditAccessFilter::isAddressRepeated(QString address, int isRange)
{
    int i;
    for (i = 0; i < m_filterList.length(); i++) {
        if (address == m_filterList.at(i).address && i != m_index) {
            if (isRange) {
                ui->lineEdit_ipRangeStart->setCustomValid(false, GET_TEXT("MYLINETIP/112011", "Already existed."));
                ui->lineEdit_ipRangeEnd->setCustomValid(false, GET_TEXT("MYLINETIP/112011", "Already existed."));
            } else {
                ui->lineEdit_ipSingle->setCustomValid(false, GET_TEXT("MYLINETIP/112011", "Already existed."));
                ui->lineEdit_mac->setCustomValid(false, GET_TEXT("MYLINETIP/112011", "Already existed."));
            }
            return 1;
        }
    }
    return 0;
}

int EditAccessFilter::check_ipv4_range_ok(QString ipStart, QString ipEnd)
{
    QStringList startList, endList;
    startList = ipStart.split(".");
    endList = ipEnd.split(".");
    QString pStr1, pStr2;
    for (int i = 0; i < 4; i++) {
        pStr1 = startList[i];
        pStr2 = endList[i];
        qDebug() << QString("i:[%1] [%2]:[%3]").arg(i).arg(pStr1).arg(pStr2);
        if (pStr2.toInt() > pStr1.toInt())
            return 1;
        if (pStr2.toInt() < pStr1.toInt())
            return 0;
    }
    return 1;
}

void EditAccessFilter::on_pushButton_ok_clicked()
{
    QString str, pstr;

    quint32 local_host = QHostAddress("127.0.0.1").toIPv4Address();

    bool valid = ui->lineEdit_mac->checkValid();
    valid = ui->lineEdit_ipSingle->checkValid() && valid;
    valid = ui->lineEdit_ipRangeStart->checkValid() && valid;
    valid = ui->lineEdit_ipRangeEnd->checkValid() && valid;
    if (!valid) {
        return;
    }

    if (ui->lineEdit_mac->isVisible()) {
        str = ui->lineEdit_mac->text();
        if (isAddressRepeated(str)) {
            return;
        }
    } else if (ui->lineEdit_ipSingle->isVisible()) {
        str = ui->lineEdit_ipSingle->text();
        str = ipAdjust(str);
        if (str == "127.0.0.1") {
            ui->lineEdit_ipSingle->setCustomValid(false, GET_TEXT("MYLINETIP/112001", "Invalid."));
            return;
        }
        if (isAddressRepeated(str)) {
            return;
        }
    } else {
        str = ui->lineEdit_ipRangeStart->text();
        pstr = ui->lineEdit_ipRangeEnd->text();
        str = ipAdjust(str);
        pstr = ipAdjust(pstr);
        quint32 startAddress = QHostAddress(str).toIPv4Address();
        quint32 endAddress = QHostAddress(pstr).toIPv4Address();
        if (local_host >= startAddress && local_host <= endAddress) {
            ui->lineEdit_ipRangeStart->setCustomValid(false, GET_TEXT("MYLINETIP/112001", "Invalid."));
            return;
        }
        if (startAddress > endAddress) {
            ui->lineEdit_ipRangeStart->setCustomValid(false, GET_TEXT("MYLINETIP/112001", "Invalid."));
            return;
        }

        str += QString("-");
        str += pstr;
        if (isAddressRepeated(str, 1)) {
            return;
        }
    }
    m_address = str;
    qMsDebug() << QString("address:[%1]").arg(m_address);
    close();
}

void EditAccessFilter::on_pushButton_cancel_clicked()
{
    m_address = QString("0");
    close();
}

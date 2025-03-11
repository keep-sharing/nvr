#include "GeneralSetting.h"
#include "ui_GeneralSetting.h"
#include "AutoLogout.h"
#include "DownloadPanel.h"
#include "EventPopup.h"
#include "LiveViewOccupancyManager.h"
#include "LogoutChannel.h"
#include "MessageBox.h"
#include "MessageFilter.h"
#include "MsApplication.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MsWaitting.h"
#include "MyDebug.h"
#include "SubControl.h"
#include "centralmessage.h"
#include "mainwindow.h"
#include "msuser.h"
#include "myqt.h"
#include "screencontroller.h"
#include "settingcontent.h"
#include <QTimer>
#include <QWSMouseHandler>
#include <QWSServer>

extern "C" {
#include "log.h"

}

enum time_type {
    AUTO_TIME = 0,
    SET_TIME,
};

GeneralSetting::GeneralSetting(QWidget *parent)
    : AbstractSettingPage(parent)
    , ui(new Ui::GeneralSetting)
{
    ui->setupUi(this);

    ui->widget_tab->addTab(GET_TEXT("SYSTEMGENERAL/70004", "Date && Time"), PageDateTime);
    ui->widget_tab->addTab(GET_TEXT("SYSTEMGENERAL/70005", "Device"), PageDevice);
    connect(ui->widget_tab, SIGNAL(tabClicked(int)), this, SLOT(onTabClicked(int)));

    ui->comboBox_mouseAccel->clear();
    ui->comboBox_mouseAccel->addItem("1", 0.2);
    ui->comboBox_mouseAccel->addItem("2", 0.3);
    ui->comboBox_mouseAccel->addItem("3", 0.5);
    ui->comboBox_mouseAccel->addItem("4", 1);
    ui->comboBox_mouseAccel->addItem(QString("5(%1)").arg(GET_TEXT("COMMON/1050", "Default")), 2);
    ui->comboBox_mouseAccel->addItem("6", 3);
    ui->comboBox_mouseAccel->addItem("7", 4);

    m_esate_status = new resp_esata_backup_status;
    memset(m_esate_status, 0, sizeof(resp_esata_backup_status));

    initGeneral();

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    m_timer->setInterval(1000);
    m_timer->start();
    onTimeout();

    connect(SubControl::instance(), SIGNAL(screenSwitched()), this, SLOT(onScreenSwitched()));

    ui->dateTimeEdit_serverTime->setDisabled(true);
    //
    if (qMsNvr->isSlaveMode()) {
        ui->label_eventPop->hide();
        ui->comboBox_eventPop->hide();
        ui->label_eventPopTime->hide();
        ui->comboBox_eventPopTime->hide();
        ui->labelOccupancyLiveView->hide();
        ui->comboBoxOccupancyLiveView->hide();
        ui->label_wizard_2->hide();
        ui->pushButton_wizard->hide();
        ui->label_wizard->hide();
        ui->comboBox_wizard->hide();
        //只能修改时区
        ui->dateTimeEdit_serverTime->setReadOnly(true);
        ui->comboBox_timeSetting->setEnabled(false);
        ui->lineEdit_ntpServer->setEnabled(false);
        ui->dateTimeEdit_setTime->setEnabled(false);
    }
    ui->lineEdit_ntpServer->setCheckMode(MyLineEdit::ServerCheck);
    ui->lineEdit_deviceName->setCheckMode(MyLineEdit::EmptyCheck);
    ui->lineEdit_interval->setCheckMode(MyLineEdit::RangeCheck, 1, 43200);

    gMessageFilter.installMessageFilter(RESPONSE_FLAG_RESOLUTION_CHANGED, this);

    onLanguageChanged();
}

GeneralSetting::~GeneralSetting()
{
    if (m_esate_status) {
        delete m_esate_status;
        m_esate_status = nullptr;
    }
    freeGeneral();
    delete ui;
}

void GeneralSetting::initializeData()
{
    gotoGeneral();
    ui->widget_tab->setCurrentTab(PageDateTime);
}

void GeneralSetting::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_SET_SYSTIME:
        ON_RESPONSE_FLAG_SET_SYSTIME(message);
        break;
    case RESPONSE_FLAG_GET_ESATA_BACKUP_STATUS:
        ON_RESPONSE_FLAG_GET_ESATA_BACKUP_STATUS(message);
        break;
    default:
        break;
    }
}

void GeneralSetting::filterMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_RESOLUTION_CHANGED:
        gotoGeneral();
        break;
    default:
        break;
    }
}

void GeneralSetting::onTabClicked(int index)
{
    ui->stackedWidget->setCurrentIndex(index);
    switch (index) {
    case 0:
        break;
    case 1:
        break;
    default:
        break;
    }
}

void GeneralSetting::ON_RESPONSE_FLAG_SET_SYSTIME(MessageReceive *message)
{
    Q_UNUSED(message)

    sendMessageOnly(REQUEST_FLAG_SYNC_IPC_TIME, NULL, 0);
    //closeWait();
}

void GeneralSetting::ON_RESPONSE_FLAG_GET_ESATA_BACKUP_STATUS(MessageReceive *message)
{
    if (message->isNull()) {
        qMsWarning() << message;
        m_eventLoop.exit(-1);
        return;
    }
    resp_esata_backup_status *info = static_cast<resp_esata_backup_status *>(message->data);
    m_eventLoop.exit(info->status);
}

QList<QPair<QString, QString>> GeneralSetting::timezoneMap()
{
    QList<QPair<QString, QString>> pairList;
    pairList.append(qMakePair(trUtf8("(UTC-11:00) United States (Samoa)"), QString("UTC11 Samoa")));
    pairList.append(qMakePair(trUtf8("(UTC-10:00) Cook Islands(Rarotonga)"), QString("UTC10 Cook")));
    pairList.append(qMakePair(trUtf8("(UTC-10:00) Tahiti"), QString("UTC10 Polynesia")));
    pairList.append(qMakePair(trUtf8("(UTC-10:00) United States (Hawaii)"), QString("UTC10 USA")));
    pairList.append(qMakePair(trUtf8("(UTC-10:00) United States (Alaska-Aleutian)"), QString("UTC10 USA2")));
    pairList.append(qMakePair(trUtf8("(UTC-09:00) United States - Alaska Time"), QString("UTC9 USA")));
    pairList.append(qMakePair(trUtf8("(UTC-08:00) Canada (Vancouver, Whitehorse)"), QString("UTC8 CAN")));
    pairList.append(qMakePair(trUtf8("(UTC-08:00) Mexico (Tijuana, Mexicali)"), QString("UTC8 MEX")));
    pairList.append(qMakePair(trUtf8("(UTC-08:00) United States - Pacific Time"), QString("UTC8 USA")));
    pairList.append(qMakePair(trUtf8("(UTC-07:00) Canada (Edmonton, Calgary)"), QString("UTC7 CAN")));
    pairList.append(qMakePair(trUtf8("(UTC-07:00) Mexico (Mazatlan, Chihuahua)"), QString("UTC7 MEX")));
    pairList.append(qMakePair(trUtf8("(UTC-07:00) United States - Mountain Time"), QString("UTC7 USA")));
    pairList.append(qMakePair(trUtf8("(UTC-06:00) Canada (Winnipeg,Edmonton)"), QString("UTC6 CAN")));
    pairList.append(qMakePair(trUtf8("(UTC-06:00) Chile (Easter Islands)"), QString("UTC6 CHL")));
    pairList.append(qMakePair(trUtf8("(UTC-06:00) Mexico (Mexico City, Acapulco)"), QString("UTC6 MEX")));
    pairList.append(qMakePair(trUtf8("(UTC-06:00) United States - Central Time"), QString("UTC6 USA")));
    pairList.append(qMakePair(trUtf8("(UTC-05:00) Bahamas (Nassau)"), QString("UTC5 BHS")));
    pairList.append(qMakePair(trUtf8("(UTC-05:00) Brazil(Acre, Amazonas)"), QString("UTC5 BRA")));
    pairList.append(qMakePair(trUtf8("(UTC-05:00) Canada (Montreal, Ottawa, Quebec)"), trUtf8("UTC5 CAN")));
    pairList.append(qMakePair(trUtf8("(UTC-05:00) Cuba (Havana)"), QString("UTC5 CUB")));
    pairList.append(qMakePair(trUtf8("(UTC-05:00) United States - Eastern Time"), QString("UTC5 USA")));
    pairList.append(qMakePair(trUtf8("(UTC-04:00) Venezuela (Caracas)"), QString("UTC4 VEN")));
    pairList.append(qMakePair(trUtf8("(UTC-04:00) Canada (Halifax, Saint John)"), QString("UTC4 CAN")));
    pairList.append(qMakePair(trUtf8("(UTC-04:00) Chile (Santiago)"), QString("UTC4 CHL")));
    pairList.append(qMakePair(trUtf8("(UTC-04:00) Paraguay (Asuncion)"), QString("UTC4 PRY")));
    pairList.append(qMakePair(trUtf8("(UTC-04:00) United Kingdom (Bermuda)"), QString("UTC4 BMU")));
    pairList.append(qMakePair(trUtf8("(UTC-04:00) United Kingdom (Falkland Islands)"), QString("UTC4 FLK")));
    pairList.append(qMakePair(trUtf8("(UTC-04:00) Trinidad&Tobago"), QString("UTC4 TTB")));
    pairList.append(qMakePair(trUtf8("(UTC-03:30) Canada (New Foundland - St. Johns)"), QString("UTC3:30 CAN")));
    pairList.append(qMakePair(trUtf8("(UTC-03:00) Denmark (Greenland - Nuuk)"), QString("UTC3 GRL")));
    pairList.append(qMakePair(trUtf8("(UTC-03:00) Argentina (Buenos Aires)"), QString("UTC3 ARG")));
    pairList.append(qMakePair(trUtf8("(UTC-03:00) Brazil (no DST)"), QString("UTC3 BRA2")));
    pairList.append(qMakePair(trUtf8("(UTC-03:00) Brazil (DST)"), QString("UTC3 BRA1")));
    pairList.append(qMakePair(trUtf8("(UTC-02:00) Brazil (no DST)"), QString("UTC2 BRA")));
    pairList.append(qMakePair(trUtf8("(UTC-01:00) Portugal (Azores)"), QString("UTC1 PRT")));
    pairList.append(qMakePair(trUtf8("(UTC+00:00) Denmark (Faroe Islands -Torshavn)"), QString("UTC0 FRO")));
    pairList.append(qMakePair(trUtf8("(UTC+00:00) Ireland (Dublin)"), QString("UTC0 IRL")));
    pairList.append(qMakePair(trUtf8("(UTC+00:00) Portugal (Lisboa, Porto, Funchal)"), QString("UTC0 PRT")));
    pairList.append(qMakePair(trUtf8("(UTC+00:00) Spain (Canary Islands - Las Palmas)"), QString("UTC0 ESP")));
    pairList.append(qMakePair(trUtf8("(UTC+00:00) United Kingdom (London)"), QString("UTC0 GBR")));
    pairList.append(qMakePair(trUtf8("(UTC+01:00) Albania (Tirane)"), QString("UTC-1 ALB")));
    pairList.append(qMakePair(trUtf8("(UTC+01:00) Austria (Vienna)"), QString("UTC-1 AUT")));
    pairList.append(qMakePair(trUtf8("(UTC+01:00) Belgium (Brussels)"), QString("UTC-1 BEL")));
    pairList.append(qMakePair(trUtf8("(UTC+01:00) Croatia (Zagreb)"), QString("UTC-1 HRV")));
    pairList.append(qMakePair(trUtf8("(UTC+01:00) Czech Republic (Prague)"), QString("UTC-1 CZE")));
    pairList.append(qMakePair(trUtf8("(UTC+01:00) Denmark (Copenhagen)"), QString("UTC-1 DNK")));
    pairList.append(qMakePair(trUtf8("(UTC+01:00) France (Nice, Paris)"), QString("UTC-1 FRA")));
    pairList.append(qMakePair(trUtf8("(UTC+01:00) Germany (Berlin)"), QString("UTC-1 GER")));
    pairList.append(qMakePair(trUtf8("(UTC+01:00) Hungary (Budapest)"), QString("UTC-1 HUN")));
    pairList.append(qMakePair(trUtf8("(UTC+01:00) Italy (Rome)"), QString("UTC-1 ITA")));
    pairList.append(qMakePair(trUtf8("(UTC+01:00) Luxembourg (Luxembourg)"), QString("UTC-1 LUX")));
    pairList.append(qMakePair(trUtf8("(UTC+01:00) Macedonia (Skopje)"), QString("UTC-1 MAK")));
    pairList.append(qMakePair(trUtf8("(UTC+01:00) Namibia (Windhoek)"), QString("UTC-1 NAM")));
    pairList.append(qMakePair(trUtf8("(UTC+01:00) Netherlands (Amsterdam)"), QString("UTC-1 NLD")));
    pairList.append(qMakePair(trUtf8("(UTC+01:00) Nigeria (Lagos)"), QString("UTC-1 NGR")));
    pairList.append(qMakePair(trUtf8("(UTC+01:00) Norway (Oslo)"), QString("UTC-1 NOR")));
    pairList.append(qMakePair(trUtf8("(UTC+01:00) Poland (Warsaw)"), QString("UTC-1 POL")));
    pairList.append(qMakePair(trUtf8("(UTC+01:00) Serbia (Begrad)"), QString("UTC-1 YUG")));
    pairList.append(qMakePair(trUtf8("(UTC+01:00) Slovakia (Bratislava)"), QString("UTC-1 SVK")));
    pairList.append(qMakePair(trUtf8("(UTC+01:00) Slovenia (Ljubljana)"), QString("UTC-1 SVN")));
    pairList.append(qMakePair(trUtf8("(UTC+01:00) Spain (Madrid, Palma)"), QString("UTC-1 ESP")));
    pairList.append(qMakePair(trUtf8("(UTC+01:00) Sweden (Stockholm)"), QString("UTC-1 SWE")));
    pairList.append(qMakePair(trUtf8("(UTC+01:00) Swiss (Bern, Zurich)"), QString("UTC-1 CHE")));
    pairList.append(qMakePair(trUtf8("(UTC+01:00) United Kingdom (Gibraltar)"), QString("UTC-1 GIB")));
    pairList.append(qMakePair(trUtf8("(UTC+01:00) West Africa Time"), QString("UTC-1 WAT")));
    pairList.append(qMakePair(trUtf8("(UTC+02:00) Bulgaria (Sofia)"), QString("UTC-2 BGR")));
    pairList.append(qMakePair(trUtf8("(UTC+02:00) Cyprus (Nicosia)"), QString("UTC-2 CYP")));
    pairList.append(qMakePair(trUtf8("(UTC+02:00) Central Africa Time"), QString("UTC-2 CAT")));
    pairList.append(qMakePair(trUtf8("(UTC+02:00) Egypt (Cairo)"), QString("UTC-2 EGY")));
    pairList.append(qMakePair(trUtf8("(UTC+02:00) Estonia (Tallinn)"), QString("UTC-2 EST")));
    pairList.append(qMakePair(trUtf8("(UTC+02:00) Finland (Helsinki)"), QString("UTC-2 FIN")));
    pairList.append(qMakePair(trUtf8("(UTC+02:00) Gaza Strip (Gaza)"), QString("UTC-2 GAZ")));
    pairList.append(qMakePair(trUtf8("(UTC+02:00) Greece (Athens)"), trUtf8("UTC-2 GRC")));
    pairList.append(qMakePair(trUtf8("(UTC+02:00) Israel (Jerusalem, Tel Aviv)"), QString("UTC-2 ISR")));
    pairList.append(qMakePair(trUtf8("(UTC+02:00) Latvia (Riga)"), QString("UTC-2 LVA")));
    pairList.append(qMakePair(trUtf8("(UTC+02:00) Lebanon (Beirut)"), QString("UTC-2 LBN")));
    pairList.append(qMakePair(trUtf8("(UTC+02:00) Moldova (Chisinau)"), QString("UTC-2 MDA")));
    pairList.append(qMakePair(trUtf8("(UTC+02:00) Palestine (Gaza)"), QString("UTC-2 PAL")));
    pairList.append(qMakePair(trUtf8("(UTC+02:00) Romania (Bucharest)"), QString("UTC-2 ROU")));
    pairList.append(qMakePair(trUtf8("(UTC+02:00) Russia (Kaliningrad)"), QString("UTC-2 RUS")));
    pairList.append(qMakePair(trUtf8("(UTC+02:00) South Africa (Johannesburg)"), QString("UTC-2 SAC")));
    pairList.append(qMakePair(trUtf8("(UTC+02:00) Syria (Damascus)"), QString("UTC-2 SYR")));
    pairList.append(qMakePair(trUtf8("(UTC+02:00) Ukraine (Kyiv, Odessa)"), QString("UTC-2 UKR")));
    pairList.append(qMakePair(trUtf8("(UTC+03:00) Belarus (Minsk)"), QString("UTC-3 BLR")));
    pairList.append(qMakePair(trUtf8("(UTC+03:00) East Africa Time"), QString("UTC-3 EAT")));
    pairList.append(qMakePair(trUtf8("(UTC+03:00) Iraq (Baghdad)"), QString("UTC-3 IRQ")));
    pairList.append(qMakePair(trUtf8("(UTC+03:00) Jordan (Amman)"), QString("UTC-3 JOR")));
    pairList.append(qMakePair(trUtf8("(UTC+03:00) Kuwait (Kuwait City)"), QString("UTC-3 AST")));
    pairList.append(qMakePair(trUtf8("(UTC+03:00) Qatar (Doha)"), QString("UTC-3 QTR")));
    pairList.append(qMakePair(trUtf8("(UTC+03:00) Russia (Moscow)"), QString("UTC-3 RUS")));
    pairList.append(qMakePair(trUtf8("(UTC+03:00) Saudi Arabia (Riyadh)"), QString("UTC-3 KSA")));
    pairList.append(qMakePair(trUtf8("(UTC+03:00) Turkey (Ankara, Istanbul)"), QString("UTC-3 TUR")));
    pairList.append(qMakePair(trUtf8("(UTC+03:30) Iran (Tehran)"), QString("UTC-3:30 IRN")));
    pairList.append(qMakePair(trUtf8("(UTC+04:00) Armenia (Yerevan)"), QString("UTC-4 ARM")));
    pairList.append(qMakePair(trUtf8("(UTC+04:00) Azerbaijan (Baku)"), QString("UTC-4 AZE")));
    pairList.append(qMakePair(trUtf8("(UTC+04:00) Georgia (Tbilisi)"), QString("UTC-4 GEO")));
    pairList.append(qMakePair(trUtf8("(UTC+04:00) Russia (Samara)"), QString("UTC-4 RUS")));
    pairList.append(qMakePair(trUtf8("(UTC+04:00) United Arab Emirates (Dubai)"), QString("UTC-4 UAE")));
    pairList.append(qMakePair(trUtf8("(UTC+05:00) Kazakhstan (Aqtau)"), QString("UTC-5 KAZ1")));
    pairList.append(qMakePair(trUtf8("(UTC+05:00) Kazakhstan (Aqtobe)"), QString("UTC-5 KAZ")));
    pairList.append(qMakePair(trUtf8("(UTC+05:00) Pakistan (Islamabad)"), QString("UTC-5 PAK")));
    pairList.append(qMakePair(trUtf8("(UTC+05:00) Russia (Chelyabinsk, Yekaterinburg)"), QString("UTC-5 RUS")));
    pairList.append(qMakePair(trUtf8("(UTC+05:30) India (Calcutta)"), QString("UTC-5:30 IND")));
    pairList.append(qMakePair(trUtf8("(UTC+05:45) Nepal (Kathmandu)"), QString("UTC-5:45 NPL")));
    pairList.append(qMakePair(trUtf8("(UTC+06:00) Kazakhstan (Astana, Almaty)"), QString("UTC-6 KAZ")));
    pairList.append(qMakePair(trUtf8("(UTC+06:00) Kyrgyzstan (Bishkek)"), QString("UTC-6 KGZ")));
    pairList.append(qMakePair(trUtf8("(UTC+06:00) Russia (Novosibirsk, Omsk)"), QString("UTC-6 RUS")));
    pairList.append(qMakePair(trUtf8("(UTC+06:30) Myanmar (Yangon)"), trUtf8("UTC-6:30 MMT")));
    pairList.append(qMakePair(trUtf8("(UTC+07:00) Russia (Krasnoyarsk)"), QString("UTC-7 RUS")));
    pairList.append(qMakePair(trUtf8("(UTC+07:00) Thailand (Bangkok)"), QString("UTC-7 THA")));
    pairList.append(qMakePair(trUtf8("(UTC+07:00) Vietnam (Ho Chi Minh City)"), QString("UTC-7 VTN")));
    pairList.append(qMakePair(trUtf8("(UTC+08:00) China (Beijing, Hong Kong, Taipei)"), QString("UTC-8 CHN")));
    pairList.append(qMakePair(trUtf8("(UTC+08:00) Singapore"), QString("UTC-8 SGP")));
    pairList.append(qMakePair(trUtf8("(UTC+08:00) Australia (Perth)"), QString("UTC-8 AUS")));
    pairList.append(qMakePair(trUtf8("(UTC+09:00) Japan (Tokyo)"), QString("UTC-9 JPN")));
    pairList.append(qMakePair(trUtf8("(UTC+09:00) Korea (Seoul)"), QString("UTC-9 KOR")));
    pairList.append(qMakePair(trUtf8("(UTC+09:30) Australia (Adelaide)"), QString("UTC-9:30 AUS")));
    pairList.append(qMakePair(trUtf8("(UTC+09:30) Australia (Darwin)"), QString("UTC-9:30 AUS2")));
    pairList.append(qMakePair(trUtf8("(UTC+10:00) Australia (Canberra, Sydney)"), QString("UTC-10 AUS")));
    pairList.append(qMakePair(trUtf8("(UTC+10:00) Australia (Brisbane)"), QString("UTC-10 AUS2")));
    pairList.append(qMakePair(trUtf8("(UTC+10:00) Australia (Hobart)"), QString("UTC-10 AUS3")));
    pairList.append(qMakePair(trUtf8("(UTC+10:00) Russia (Vladivostok)"), QString("UTC-10 RUS")));
    pairList.append(qMakePair(trUtf8("(UTC+10:30) Australia (Lord Howe Island)"), QString("UTC-10:30 AUS")));
    pairList.append(qMakePair(trUtf8("(UTC+11:00) New Caledonia (Noumea)"), QString("UTC-11 NCL")));
    pairList.append(qMakePair(trUtf8("(UTC+12:00) New Zealand (Wellington, Auckland)"), QString("UTC-12 NZL")));
    pairList.append(qMakePair(trUtf8("(UTC+12:00) Russia (Anadyr, Kamchatka)"), QString("UTC-12 RUS")));
    pairList.append(qMakePair(trUtf8("(UTC+12:45) New Zealand (Chatham Islands)"), QString("UTC-12:45 NZL")));
    pairList.append(qMakePair(trUtf8("(UTC+13:00) Tonga (Nukualofa)"), QString("UTC-13 TON")));
    return pairList;
}

void GeneralSetting::initGeneral()
{
    qDebug() << "====GeneralSetting::initGeneral====";

    //
    const struct device_info &sys_info = qMsNvr->deviceInfo();

    qDebug() << "----max_screen:" << sys_info.max_screen;
    qDebug() << "----model:" << sys_info.model;
    qDebug() << "----prefix:" << sys_info.prefix;

    //Device Name
    QRegExp rxDeviceName(QString("[a-zA-Z0-9]*"));
    QValidator *validator = new QRegExpValidator(rxDeviceName, this);
    ui->lineEdit_deviceName->setValidator(validator);

    //
    ui->label_hdmi_vga_output->setTranslatableText("SYSTEMGENERAL/70061", "HDMI/VGA Output");
    ui->label_hdmi_vga_output->hide();
    ui->comboBox_hdmi_vga_output->hide();
    ui->comboBox_hdmi_vga_output->clear();
    ui->comboBox_hdmi_vga_output->addTranslatableItem("SYSTEMGENERAL/70056", "Independent", 0);
    ui->comboBox_hdmi_vga_output->addTranslatableItem("SYSTEMGENERAL/70055", "Synchronous", 1);

    //
    QMap<int, QString> resolutionMap;
    resolutionMap.insert(OUTPUT_RESOLUTION_1080P, "1920 x 1080 / 60Hz");
    resolutionMap.insert(OUTPUT_RESOLUTION_720P, "1280 x 720 / 60Hz");
    resolutionMap.insert(OUTPUT_RESOLUTION_SXGA, "1280 x 1024");
    resolutionMap.insert(OUTPUT_RESOLUTION_XGA, "1024 x 768");
    resolutionMap.insert(OUTPUT_RESOLUTION_1080P50, "1920 x 1080 / 50Hz");
    resolutionMap.insert(OUTPUT_RESOLUTION_2160P30, "3840 x 2160 / 30Hz");
    resolutionMap.insert(OUTPUT_RESOLUTION_2160P60, "3840 x 2160 / 60Hz");
    ui->comboBox_screen1->clear();
    ui->comboBox_screen2->clear();
    for (auto iter = resolutionMap.constBegin(); iter != resolutionMap.constEnd(); ++iter) {
        int value = iter.key();
        QString text = iter.value();
        ui->comboBox_screen1->addItem(text, value);
        ui->comboBox_screen2->addItem(text, value);
    }
    //
    ui->comboBox_screen2->removeItemFromData(OUTPUT_RESOLUTION_2160P60);
    ui->comboBox_screen2->removeItemFromData(OUTPUT_RESOLUTION_2160P30);

    //
    QString strModel(sys_info.model);
    QString strPrefix(sys_info.prefix);
    qDebug()<<"Gsjt3"<<qMsNvr->multiScreenSupport();
    if (qMsNvr->multiScreenSupport() == 2) {
        ui->label_hdmi_vga_output->show();
        ui->comboBox_hdmi_vga_output->show();
        ui->label_screen2_enable->hide();
        ui->comboBox_screen2_enable->hide();
        ui->label_hdmiAudio->setTranslatableText("SYSTEMGENERAL/70039", "HDMI Audio");

        ui->comboBox_eventPop->clear();
        ui->comboBox_eventPop->addItem("HDMI", 0);
        ui->comboBox_eventPop->addItem("VGA", 1);
        ui->comboBox_eventPop->addTranslatableItem("ANPR/103038", "All", 2);

        ui->comboBoxOccupancyLiveView->clear();
        ui->comboBoxOccupancyLiveView->addItem("HDMI", 0);
        ui->comboBoxOccupancyLiveView->addItem("VGA", 1);
        ui->comboBoxOccupancyLiveView->addTranslatableItem("ANPR/103038", "All", 2);

        //这个功能仅3536平台，一个HDMI+一个VGA的型号支持（MS-N7016-UH，MS-N7032-UH，MS-N7016-UPH，MS-N7032-UPH）
        m_homologous = get_param_int(SQLITE_FILE_NAME, PARAM_HOMOLOGOUS, 1);
        ui->comboBox_hdmi_vga_output->setCurrentIndexFromData(m_homologous);
        switch (m_homologous) {
        case 0:
            //如果显示Independent，为HDMI和VGA异源输出，HDMI为主，VGA为辅
            ui->label_screen1->setTranslatableText("SYSTEMGENERAL/70057", "HDMI Resolution");
            ui->label_screen2->setTranslatableText("SYSTEMGENERAL/70058", "VGA Resolution");
            break;
        case 1:
            //如果选择Synchronous，为HDMI和VGA同源输出
            ui->label_screen1->setTranslatableText("SYSTEMGENERAL/70054", "HDMI/VGA Resolution");
            ui->label_screen2->hide();
            ui->comboBox_screen2->hide();
            break;
        }
    } else if (qMsNvr->multiScreenSupport() == 1) {
        ui->label_screen1->setTranslatableText("SYSTEMGENERAL/70011", "HDMI1/VGA1 Resolution");
        ui->label_screen2->setTranslatableText("SYSTEMGENERAL/70014", "HDMI2/VGA2 Resolution");
        ui->label_hdmiAudio->setTranslatableText("SYSTEMGENERAL/70015", "HDMI1 Audio");

        ui->comboBox_eventPop->clear();
        ui->comboBox_eventPop->addTranslatableItem("SYSTEMGENERAL/70010", "HDMI1/VGA1", 0);
        ui->comboBox_eventPop->addTranslatableItem("SYSTEMGENERAL/70013", "HDMI2/VGA2", 1);
        ui->comboBox_eventPop->addTranslatableItem("ANPR/103038", "All", 2);

        ui->comboBoxOccupancyLiveView->clear();
        ui->comboBoxOccupancyLiveView->addTranslatableItem("SYSTEMGENERAL/70010", "HDMI1/VGA1", 0);
        ui->comboBoxOccupancyLiveView->addTranslatableItem("SYSTEMGENERAL/70013", "HDMI2/VGA2", 1);
        ui->comboBoxOccupancyLiveView->addTranslatableItem("ANPR/103038", "All", 2);
    } else {
        ui->label_screen1->setTranslatableText("SYSTEMGENERAL/70054", "HDMI/VGA Resolution");
        ui->label_screen2_enable->hide();
        ui->comboBox_screen2_enable->hide();
        ui->label_screen2->hide();
        ui->comboBox_screen2->hide();
        ui->label_hdmiAudio->setTranslatableText("SYSTEMGENERAL/70039", "HDMI Audio");
        //
        ui->label_screenSwitch->hide();
        ui->comboBox_screenSwitch->hide();
        ui->label_eventPop->hide();
        ui->comboBox_eventPop->hide();
        ui->labelOccupancyLiveView->hide();
        ui->comboBoxOccupancyLiveView->hide();
        //
        if (strModel == QString("MS-N5016-UPT") || strModel == QString("MS-N5016-UT") || strModel == QString("MS-N5008-UPT") || strModel == QString("MS-N5008-UT")) {
            ui->comboBox_screen1->removeItemFromData(OUTPUT_RESOLUTION_2160P60);
        }
        if (strPrefix == QString("1")) {
            ui->comboBox_screen1->removeItemFromData(OUTPUT_RESOLUTION_2160P60);
        }
        if (qMsNvr->is3536c()) {
            ui->comboBox_screen1->removeItemFromData(OUTPUT_RESOLUTION_2160P60);
        }
#if defined(_NT98323_) || defined(_NT98633_)
        ui->comboBox_screen1->removeItemFromData(OUTPUT_RESOLUTION_2160P60);
#endif
    }

    //
    ui->comboBox_audioVolume->clear();
    ui->label_audioVolume->hide();
    ui->comboBox_audioVolume->hide();

    //
    ui->comboBox_menuTimeout->clear();
    ui->comboBox_menuTimeout->addTranslatableItem("COMMON/1018", "Disable", 0);
    ui->comboBox_menuTimeout->addTranslatableItem("", "30s", 30);
    ui->comboBox_menuTimeout->addTranslatableItem("", "60s", 60);
    ui->comboBox_menuTimeout->addTranslatableItem("", "120s", 120);
    ui->comboBox_menuTimeout->addTranslatableItem("", "240s", 240);
    ui->comboBox_menuTimeout->addTranslatableItem("", "360s", 360);

    ui->comboBoxAutoLogout->clear();
    ui->comboBoxAutoLogout->addTranslatableItem("COMMON/1018", "Disable", 0);
    ui->comboBoxAutoLogout->addTranslatableItem("", "30s", 30);
    ui->comboBoxAutoLogout->addTranslatableItem("", "60s", 60);
    ui->comboBoxAutoLogout->addTranslatableItem("", "120s", 120);
    ui->comboBoxAutoLogout->addTranslatableItem("", "240s", 240);
    ui->comboBoxAutoLogout->addTranslatableItem("", "360s", 360);

    //
    MsLanguage::instance()->initializeComboBox(ui->comboBox_language);
    old_language = MsLanguage::instance()->currentLanguage();

    //
    if (sys_info.max_audio_out == 0) {
        ui->label_audioEnable->hide();
        ui->comboBox_audioEnable->hide();
    }

    //
    char mouseAccel[10] = { 0 };
    get_param_value(SQLITE_FILE_NAME, "mouse_accel", mouseAccel, sizeof(mouseAccel), "2");
    double mouse = QString(mouseAccel).toDouble();
    ui->comboBox_mouseAccel->setCurrentIndexFromData(mouse);

    initTime();

    //
    ui->label_screen2_enable->setTranslatableText("SYSTEMGENERAL/70013", "HDMI2/VGA2");
    ui->comboBox_screen2_enable->clear();
    ui->comboBox_screen2_enable->addTranslatableItem("COMMON/1018", "Disable", 0);
    ui->comboBox_screen2_enable->addTranslatableItem("COMMON/1009", "Enable", 1);

    ui->comboBox_hdmiAudio->clear();
    ui->comboBox_hdmiAudio->addTranslatableItem("COMMON/1018", "Disable", 0);
    ui->comboBox_hdmiAudio->addTranslatableItem("COMMON/1009", "Enable", 1);

    ui->comboBox_audioEnable->clear();
    ui->comboBox_audioEnable->addTranslatableItem("COMMON/1018", "Disable", 0);
    ui->comboBox_audioEnable->addTranslatableItem("COMMON/1009", "Enable", 1);

    ui->comboBox_wizard->clear();
    ui->comboBox_wizard->addTranslatableItem("COMMON/1018", "Disable", 0);
    ui->comboBox_wizard->addTranslatableItem("COMMON/1009", "Enable", 1);

    ui->comboBox_localAuth->clear();
    ui->comboBox_localAuth->addTranslatableItem("COMMON/1018", "Disable", 0);
    ui->comboBox_localAuth->addTranslatableItem("COMMON/1009", "Enable", 1);

    ui->comboBox_menuAuth->clear();
    ui->comboBox_menuAuth->addTranslatableItem("COMMON/1018", "Disable", 0);
    ui->comboBox_menuAuth->addTranslatableItem("COMMON/1009", "Enable", 1);

    ui->comboBox_timeSetting->clear();
    ui->comboBox_timeSetting->addTranslatableItem("WIZARD/11018", "NTP Server");
    ui->comboBox_timeSetting->addTranslatableItem("NETWORKSTATUS/61008", "Manual");

    ui->comboBox_eventPopTime->clear();
    for (int i = 1; i <= 11; i++) {
        ui->comboBox_eventPopTime->addItem(GET_TEXT("RECORDADVANCE/91007", "%1s").arg(i), i);
    }
    ui->comboBox_eventPopTime->addTranslatableItem("EVENTSTATUS/63011", "Manually Clear", 0);

    ui->comboBox_dsTime->clear();
    ui->comboBox_dsTime->addTranslatableItem("COMMON/1018", "Disable");
    ui->comboBox_dsTime->addTranslatableItem("COMMON/1014", "Auto");

    ui->comboBox_ntpSync->clear();
    ui->comboBox_ntpSync->addTranslatableItem("COMMON/1018", "Disable", 0);
    ui->comboBox_ntpSync->addTranslatableItem("COMMON/1009", "Enable", 1);

    ui->label_mouseAccel->setTranslatableText("SYSTEMGENERAL/70060", "Mouse Pointer Speed Level");
}

void GeneralSetting::initTime()
{
    ui->comboBox_timezone->clear();
    const auto &zoneList = timezoneMap();
    for (int i = 0; i < zoneList.size(); ++i) {
        const QPair<QString, QString> &pair = zoneList.at(i);
        const QString &key = pair.first;
        const QString &value = pair.second;
        ui->comboBox_timezone->addItem(key, value);
    }
    connect(ui->comboBox_timeSetting, SIGNAL(currentIndexChanged(int)), this, SLOT(slotTimeSettingChanged(int)));
}

void GeneralSetting::gotoGeneral()
{
    if (pDisplayDb == NULL) {
        pDisplayDb = new display;
    }
    if (pTimeDb == NULL) {
        pTimeDb = new struct time;
    }
    if (pTimeDbOri == NULL) {
        pTimeDbOri = new struct time;
    }
    if (pDisplayDb == NULL || pTimeDb == NULL || pTimeDbOri == NULL) {
        return;
    }
    memset(pTimeDb, 0, sizeof(struct time));
    memset(pTimeDbOri, 0, sizeof(struct time));
    read_time(SQLITE_FILE_NAME, pTimeDb);
    memcpy(pTimeDbOri, pTimeDb, sizeof(struct time));

    readConfig();

    if (SubControl::instance()->isSubControl()) {
        ui->comboBox_screen1->setEnabled(false);
        ui->comboBox_screen2_enable->setEnabled(false);
        ui->comboBox_screen2->setEnabled(true);
    } else {
        ui->comboBox_screen1->setEnabled(true);
        ui->comboBox_screen2_enable->setEnabled(true);
        ui->comboBox_screen2->setEnabled(false);
    }
}

void GeneralSetting::freeGeneral()
{
    if (pTimeDb) {
        delete pTimeDb;
        pTimeDb = nullptr;
    }
    if (pTimeDbOri) {
        delete pTimeDbOri;
        pTimeDbOri = nullptr;
    }
    if (pDisplayDb) {
        delete pDisplayDb;
        pDisplayDb = nullptr;
    }
}

void GeneralSetting::readConfig()
{
    if (!pDisplayDb) {
        return;
    }
    memset(pDisplayDb, 0, sizeof(display));
    qMsNvr->readDisplayInfo();
    *pDisplayDb = qMsNvr->displayInfo();

    ui->comboBox_screen1->setCurrentIndexFromData(pDisplayDb->main_resolution);
    ui->comboBox_screen2->setCurrentIndexFromData(pDisplayDb->sub_resolution);

    ui->comboBox_screen2_enable->setCurrentIndex(pDisplayDb->sub_enable);
    ui->comboBox_screen2->setEnabled(pDisplayDb->sub_enable);

    ui->comboBox_screenSwitch->setEnabled(pDisplayDb->sub_enable);
    ui->comboBox_screenSwitch->setCurrentIndex(qMsNvr->isQuickSwitchScreenEnable());

    if (pDisplayDb->audio_output_on == 0) {
        ui->comboBox_audioEnable->setCurrentIndex(0);
    } else {
        ui->comboBox_audioEnable->setCurrentIndex(1);
    }

    if (pDisplayDb->hdmi_audio_on == 0) {
        ui->comboBox_hdmiAudio->setCurrentIndex(0);
    } else {
        ui->comboBox_hdmiAudio->setCurrentIndex(1);
    }

    ui->comboBox_audioVolume->setCurrentIndex(ui->comboBox_audioVolume->findData(pDisplayDb->volume));

    if (pDisplayDb->sub_enable) {
        ui->comboBox_eventPop->setEnabled(true);
        ui->comboBox_eventPop->setCurrentIndex(pDisplayDb->eventPop_screen);
        ui->comboBoxOccupancyLiveView->setEnabled(true);
        ui->comboBoxOccupancyLiveView->setCurrentIndex(pDisplayDb->occupancy_screen);
    } else {
        ui->comboBox_eventPop->setEnabled(false);
        ui->comboBox_eventPop->setCurrentIndex(0);
        ui->comboBoxOccupancyLiveView->setEnabled(false);
        ui->comboBoxOccupancyLiveView->setCurrentIndex(0);
    }

    ui->comboBox_eventPopTime->setCurrentIndex(ui->comboBox_eventPopTime->findData(pDisplayDb->eventPop_time));

    {
        char tmp[20] = { 0 };
        get_param_value(SQLITE_FILE_NAME, PARAM_GUI_WIZARD_ENABLE, tmp, sizeof(tmp), "");
        int enable = atoi(tmp);
        if (enable) {
            ui->comboBox_wizard->setCurrentIndex(1);
        } else {
            ui->comboBox_wizard->setCurrentIndex(0);
        }
    }
    {
        struct network netDb;
        memset(&netDb, 0, sizeof(netDb));
        read_network(SQLITE_FILE_NAME, &netDb);
        ui->lineEdit_deviceName->setText(QString(netDb.host_name));
    }

    {
        char tmp[20] = { 0 };
        get_param_value(SQLITE_FILE_NAME, PARAM_GUI_AUTH, tmp, sizeof(tmp), "");
        _localauth_enable = atoi(tmp);
        ui->comboBox_localAuth->setCurrentIndex(_localauth_enable);
        if (!gMsUser.isAdmin()) {
            ui->label_localAuth->hide();
            ui->comboBox_localAuth->hide();
        } else {
            ui->label_localAuth->show();
            ui->comboBox_localAuth->show();
        }
    }
    {
        char tmp[20] = { 0 };
        get_param_value(SQLITE_FILE_NAME, PARAM_GUI_MENU_AUTH, tmp, sizeof(tmp), "");
        _menuauth_enable = atoi(tmp);
        ui->comboBox_menuAuth->setCurrentIndex(_menuauth_enable);
    }
    {
        char tmp[20] = { 0 };
        get_param_value(SQLITE_FILE_NAME, PARAM_GUI_MENU_TIME_OUT, tmp, sizeof(tmp), "");
        _menu_timeout = atoi(tmp);
        ui->comboBox_menuTimeout->setCurrentIndex(ui->comboBox_menuTimeout->findData(_menu_timeout));
    }
    {
        _sys_language = MsLanguage::instance()->currentLanguage();
        ui->comboBox_language->setCurrentIndex(ui->comboBox_language->findData(_sys_language));
    }
    ui->comboBoxAutoLogout->setCurrentIndexFromData(gAutoLogout.readInterval());

    //time setting
    char timezone[64];
    snprintf(timezone, sizeof(timezone), "%s %s", pTimeDb->time_zone, pTimeDb->time_zone_name);
    int index = ui->comboBox_timezone->findData(QString(timezone));
    old_zone_index = index;
    ui->dateTimeEdit_setTime->setDateTime(QDateTime::currentDateTime());
    ui->comboBox_timezone->setCurrentIndex(index);
    ui->comboBox_dsTime->setCurrentIndex(pTimeDb->dst_enable);

    if (pTimeDb->ntp_enable) {
        ui->comboBox_timeSetting->setCurrentIndex(AUTO_TIME);
        slotTimeSettingChanged(AUTO_TIME);
    } else {
        ui->comboBox_timeSetting->setCurrentIndex(SET_TIME);
        slotTimeSettingChanged(SET_TIME);
    }
    ui->lineEdit_ntpServer->setText(QString(pTimeDb->ntp_server));

    ui->comboBox_ntpSync->setCurrentIndex(pTimeDbOri->sync_enable);
    ui->lineEdit_interval->setText(QString("%1").arg(pTimeDbOri->sync_interval));
    on_comboBox_ntpSync_activated(pTimeDbOri->sync_enable);

    ui->comboBox_hdmi_vga_output->setCurrentIndexFromData(m_homologous);
}

void GeneralSetting::saveConfig()
{
}

void GeneralSetting::saveLanguage()
{
    int id = ui->comboBox_language->currentData().toInt();
    _sys_language = id;
    MsLanguage::instance()->changeLanguage(id);
}

void GeneralSetting::setMainSubResolution(int main_resolution, int sub_resolution, int spot_resolution)
{
    Q_UNUSED(spot_resolution)

    REQ_SCREEN_S screen;
    screen.enScreen = SCREEN_MAIN;
    screen.enRes = (DisplayDcMode_e)main_resolution;
    Q_UNUSED(screen)
    sendMessageOnly(REQUEST_FLAG_SET_SCREEN, (void *)&screen, sizeof(screen));

    screen.enScreen = SCREEN_SUB;
    screen.enRes = (DisplayDcMode_e)sub_resolution;
    sendMessageOnly(REQUEST_FLAG_SET_SCREEN, (void *)&screen, sizeof(screen));

    //MainWindow::instance()->hide();
    //ScreenController::instance()->blackScreen(3000);
    //ScreenController::instance()->setRefreshRate(DisplayDcMode_e(SubControl::instance()->currentScreen() == SCREEN_MAIN ? main_resolution : sub_resolution));
}

void GeneralSetting::onLanguageChanged()
{
    ui->widget_tab->setTabText(PageDateTime, GET_TEXT("SYSTEMGENERAL/70004", "Date && Time"));
    ui->widget_tab->setTabText(PageDevice, GET_TEXT("SYSTEMGENERAL/70005", "Device"));

    ui->label_deviceName->setText(GET_TEXT("SYSTEMGENERAL/70009", "Device Name"));

    ui->label_hdmi_vga_output->retranslate();
    ui->comboBox_hdmi_vga_output->retranslate();
    ui->label_screen1->retranslate();
    ui->label_screen2_enable->retranslate();
    ui->comboBox_screen2_enable->retranslate();
    ui->label_screen2->retranslate();

    ui->label_hdmiAudio->retranslate();
    ui->comboBox_hdmiAudio->retranslate();

    ui->label_screenSwitch->setText(GET_TEXT("SYSTEMGENERAL/70028", "Quick Screen Switch"));
    ui->comboBox_screenSwitch->setItemText(0, GET_TEXT("COMMON/1018", "Disable"));
    ui->comboBox_screenSwitch->setItemText(1, GET_TEXT("COMMON/1009", "Enable"));

    ui->comboBox_audioEnable->retranslate();
    ui->comboBox_wizard->retranslate();
    ui->comboBox_localAuth->retranslate();
    ui->comboBox_menuAuth->retranslate();
    ui->comboBox_timeSetting->retranslate();
    ui->comboBox_eventPopTime->retranslate();

    ui->label_audioEnable->setText(GET_TEXT("SYSTEMGENERAL/70016", "Audio Out"));
    ui->label_wizard->setText(GET_TEXT("SYSTEMGENERAL/70017", "Boot Wizard"));
    ui->label_localAuth->setText(GET_TEXT("SYSTEMGENERAL/70018", "Boot Authentication"));
    ui->label_menuAuth->setText(GET_TEXT("SYSTEMGENERAL/70019", "Menu Authentication"));
    ui->label_menuTimeout->setText(GET_TEXT("SYSTEMGENERAL/70020", "Settings Page Timeout"));
    ui->label_language->setText(GET_TEXT("WIZARD/11011", "Language"));
    ui->label_serverTime->setText(GET_TEXT("SYSTEMGENERAL/70024", "Current System Time"));
    ui->label_timezone->setText(GET_TEXT("WIZARD/11014", "Time Zone"));
    ui->label_dsTime->setText(GET_TEXT("WIZARD/11016", "Daylight Saving Time"));
    ui->label_timeSetting->setText(GET_TEXT("CHANNELMANAGE/30018", "Time Setting"));
    ui->label_ntpServer->setText(GET_TEXT("SYSTEMGENERAL/70027", "Server Address"));
    ui->label_setTime->setText(GET_TEXT("AUTOREBOOT/78010", "Time"));
    ui->label_wizard_2->setText(GET_TEXT("SYSTEMGENERAL/70021", "Wizard"));
    ui->pushButton_wizard->setText(GET_TEXT("SYSTEMGENERAL/70023", "Start"));
    ui->label_ntpSync->setText(GET_TEXT("SYSTEMGENERAL/70025", "NTP Sync"));
    ui->label_interval->setText(GET_TEXT("SYSTEMGENERAL/70026", "Interval"));
    ui->comboBox_mouseAccel->setItemText(4, QString("5(%1)").arg(GET_TEXT("COMMON/1050", "Default")));
    ui->comboBox_menuTimeout->retranslate();
    ui->labelAutoLogout->setText(GET_TEXT("SYSTEMGENERAL/70067", "Auto Logout"));
    ui->comboBoxAutoLogout->retranslate();
    ui->labelLogoutChannel->setText(GET_TEXT("SYSTEMGENERAL/70068", "Display When Logout"));
    ui->pushButtonLogoutChannel->setText(GET_TEXT("COMMON/1019", "Edit"));

    ui->comboBox_dsTime->retranslate();

    ui->comboBox_eventPop->retranslate();
    ui->comboBoxOccupancyLiveView->retranslate();

    ui->label_eventPop->setText(GET_TEXT("SYSTEMGENERAL/70041", "Event Popup"));
    ui->label_eventPopTime->setText(GET_TEXT("SYSTEMGENERAL/70042", "Event Popup Duration Time"));
    ui->labelOccupancyLiveView->setText(GET_TEXT("OCCUPANCY/74214", "Occupancy Live View"));

    ui->label_mouseAccel->retranslate();

    ui->comboBox_ntpSync->retranslate();

    ui->pushButton_apply->setText(GET_TEXT("COMMON/1003", "Apply"));
    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));
    ui->pushButton_apply_2->setText(GET_TEXT("COMMON/1003", "Apply"));
    ui->pushButton_back_2->setText(GET_TEXT("COMMON/1002", "Back"));
}

void GeneralSetting::slotTimeSettingChanged(int index)
{
    if (index == AUTO_TIME) {
        ui->label_ntpServer->show();
        ui->lineEdit_ntpServer->show();
        ui->label_setTime->hide();
        ui->dateTimeEdit_setTime->hide();
        ui->label_ntpSync->show();
        ui->comboBox_ntpSync->show();
        ui->label_interval->show();
        ui->widgetInterval->show();

    } else {
        ui->label_ntpServer->hide();
        ui->lineEdit_ntpServer->hide();
        ui->label_setTime->show();
        ui->dateTimeEdit_setTime->show();
        ui->label_ntpSync->hide();
        ui->comboBox_ntpSync->hide();
        ui->label_interval->hide();
        ui->widgetInterval->hide();
    }
}

void GeneralSetting::on_comboBox_hdmi_vga_output_activated(int index)
{
    bool multiEnable = (index == 0);
    if (multiEnable) {
        ui->comboBox_screenSwitch->setCurrentIndex(1);
    } else {
        ui->comboBox_screenSwitch->setCurrentIndex(0);
    }

    ui->comboBox_screenSwitch->setEnabled(multiEnable);

    if (multiEnable) {
        ShowMessageBox(GET_TEXT("SYSTEMGENERAL/70029", "Double-click the mouse wheel to switch Main/Sub screen control!"));
    }
}

void GeneralSetting::on_comboBox_screen2_enable_activated(int index)
{
    bool enable = (index == 1);
    if (enable && !pDisplayDb->sub_enable) {
        ui->comboBox_screen2->setEnabled(true);
        ui->comboBox_eventPop->setEnabled(true);
        ui->comboBox_eventPop->setCurrentIndex(pDisplayDb->eventPop_screen);
        ui->comboBoxOccupancyLiveView->setEnabled(true);
        ui->comboBoxOccupancyLiveView->setCurrentIndex(pDisplayDb->occupancy_screen);
    } else {
        ui->comboBox_screen2->setEnabled(false);
        ui->comboBox_eventPop->setEnabled(false);
        ui->comboBox_eventPop->setCurrentIndex(0);
        ui->comboBoxOccupancyLiveView->setEnabled(false);
        ui->comboBoxOccupancyLiveView->setCurrentIndex(0);
    }
    if (enable) {
        ui->comboBox_eventPop->setEnabled(true);
        ui->comboBox_eventPop->setCurrentIndex(pDisplayDb->eventPop_screen);
        ui->comboBoxOccupancyLiveView->setEnabled(true);
        ui->comboBoxOccupancyLiveView->setCurrentIndex(pDisplayDb->occupancy_screen);

        ui->comboBox_screenSwitch->setCurrentIndex(1);
    } else {
        ui->comboBox_eventPop->setEnabled(false);
        ui->comboBox_eventPop->setCurrentIndex(0);
        ui->comboBoxOccupancyLiveView->setEnabled(false);
        ui->comboBoxOccupancyLiveView->setCurrentIndex(0);

        ui->comboBox_screenSwitch->setCurrentIndex(0);
    }

    ui->comboBox_screenSwitch->setEnabled(enable);

    if (enable) {
        ShowMessageBox(GET_TEXT("SYSTEMGENERAL/70029", "Double-click the mouse wheel to switch Main/Sub screen control!"));
    }
}

void GeneralSetting::onScreenSwitched()
{
    if (SubControl::instance()->isSubControl()) {
        ui->comboBox_screen2_enable->setEnabled(false);
        ui->comboBox_screen1->setEnabled(false);
        ui->comboBox_screen2->setEnabled(true);
    } else {
        ui->comboBox_screen2_enable->setEnabled(true);
        ui->comboBox_screen1->setEnabled(true);
        ui->comboBox_screen2->setEnabled(false);
    }
}

void GeneralSetting::onTimeout()
{
    ui->dateTimeEdit_serverTime->setDateTime(QDateTime::currentDateTime());
}

void GeneralSetting::on_pushButton_apply_clicked()
{
    qDebug() << "====GeneralSetting::on_pushButton_apply_clicked====";
    int wantLogWrite = 0;

    QString strDeviceName = ui->lineEdit_deviceName->text().trimmed();
    qDebug() << ui->lineEdit_deviceName->text();
    if (!ui->lineEdit_deviceName->checkValid()) {
        return;
    }

    struct network netDb;
    memset(&netDb, 0, sizeof(netDb));
    read_network(SQLITE_FILE_NAME, &netDb);
    int hostNameChanged = 0, subEnableChanged = 0, isChanged = 0;
    if (QString(netDb.host_name) != strDeviceName) {
        snprintf(netDb.host_name, sizeof(netDb.host_name), "%s", strDeviceName.toStdString().c_str());
        hostNameChanged = 1;
    }

    if (hostNameChanged) {
        write_network(SQLITE_FILE_NAME, &netDb);

        struct req_set_sysconf timeconf;
        memset(&timeconf, 0, sizeof(timeconf));
        snprintf(timeconf.arg, sizeof(timeconf.arg), "%s", "hostname");
        sendMessageOnly(REQUEST_FLAG_SET_NETWORK, (void *)&timeconf, sizeof(timeconf));
        wantLogWrite = 1;
    }

    int screen2Enable = ui->comboBox_screen2_enable->currentData().toInt();
    if (pDisplayDb->sub_enable != screen2Enable) {
        pDisplayDb->sub_enable = screen2Enable;
        subEnableChanged = 1;
        isChanged = 1;
        ui->comboBox_screen2->setEnabled(false);
    }

    int screen1Resolution = ui->comboBox_screen1->currentData().toInt();
    int screen2Resolution = ui->comboBox_screen2->currentData().toInt();
    if (pDisplayDb->main_resolution != screen1Resolution || pDisplayDb->sub_resolution != screen2Resolution) {
        setMainSubResolution(screen1Resolution, screen2Resolution, pDisplayDb->spot_resolution);
#if defined(_HI3536A_)
        int timeout = 30;
#else
        int timeout = 20;
#endif
        int result = MessageBox::timeoutQuestion(this, GET_TEXT("SYSTEMGENERAL/70037", "Modify Resolution"), GET_TEXT("SYSTEMGENERAL/70038", "Are you sure to modify the resolution ?"), timeout);
        if (result == MessageBox::Yes) {
            pDisplayDb->main_resolution = screen1Resolution;
            pDisplayDb->sub_resolution = screen2Resolution;
            isChanged = 1;
            //qreal accel = ui->comboBox_mouseAccel->currentData().toDouble();
            //ScreenController::instance()->setMouseSpeed(accel);
        } else {
            setMainSubResolution(pDisplayDb->main_resolution, pDisplayDb->sub_resolution, pDisplayDb->spot_resolution);
            ui->comboBox_screen1->setCurrentIndexFromData(pDisplayDb->main_resolution);
            ui->comboBox_screen2->setCurrentIndexFromData(pDisplayDb->sub_resolution);
        }
        ui->comboBox_screen1->setFocus();
        sendMessageOnly(REQUEST_FLAG_SET_SCREEN_END, (void *)&isChanged, sizeof(isChanged));
    }

    //audio out / hdmi0
    int volume = 0;
    volume = ui->comboBox_audioVolume->itemData(ui->comboBox_audioVolume->currentIndex()).toInt();

    if (pDisplayDb->audio_output_on != ui->comboBox_audioEnable->currentIndex()
        || pDisplayDb->hdmi_audio_on != ui->comboBox_hdmiAudio->currentIndex()
        || pDisplayDb->volume != volume) {
        pDisplayDb->audio_output_on = ui->comboBox_audioEnable->currentIndex();
        pDisplayDb->hdmi_audio_on = ui->comboBox_hdmiAudio->currentIndex();

        pDisplayDb->volume = volume;

        isChanged = 1;
    }

    //popup
    if (ui->comboBox_eventPop->isVisible()) {
        int screen = ui->comboBox_eventPop->currentIndex();
        if (pDisplayDb->eventPop_screen != screen) {
            isChanged = 1;
            pDisplayDb->eventPop_screen = screen;
        }
    }
    if (pDisplayDb->eventPop_time != ui->comboBox_eventPopTime->itemData(ui->comboBox_eventPopTime->currentIndex()).toInt()) {
        isChanged = 1;
        pDisplayDb->eventPop_time = ui->comboBox_eventPopTime->itemData(ui->comboBox_eventPopTime->currentIndex()).toInt();
    }
    if (ui->comboBoxOccupancyLiveView->isVisible()) {
        int screen = ui->comboBoxOccupancyLiveView->currentIndex();
        if (pDisplayDb->occupancy_screen != screen) {
            isChanged = 1;
            pDisplayDb->occupancy_screen = screen;
            LiveViewOccupancyManager::instance()->setScreen(screen);
        }
    }
    if (isChanged) {
        EventPopup::instance()->setPopupInfo(pDisplayDb->eventPop_screen, pDisplayDb->eventPop_time);

        qMsNvr->writeDisplayInfo(pDisplayDb);
        wantLogWrite = 1;

        //开启、关闭辅屏
        if (subEnableChanged) {
            REFRESH_E enRefresh;
            if (pDisplayDb->sub_enable == 0) {
                enRefresh = REFRESH_CLEAR_VDEC;
                Q_UNUSED(enRefresh)

                //SubControl::instance()->setSubEnable(false);
            } else {
                //SubControl::instance()->setSubEnable(true);
            }
        }
    }
    {
        char tmp[20] = { 0 };
        get_param_value(SQLITE_FILE_NAME, PARAM_GUI_WIZARD_ENABLE, tmp, sizeof(tmp), "");
        int enable = atoi(tmp);
        if (ui->comboBox_wizard->currentIndex() != enable) {
            enable = ui->comboBox_wizard->currentIndex();
            snprintf(tmp, sizeof(tmp), "%d", enable);
            set_param_value(SQLITE_FILE_NAME, PARAM_GUI_WIZARD_ENABLE, tmp);
            wantLogWrite = 1;
        }
    }
    if (_localauth_enable != ui->comboBox_localAuth->currentIndex()) {
        _localauth_enable = ui->comboBox_localAuth->currentIndex();
        char tmp[20] = { 0 };
        snprintf(tmp, sizeof(tmp), "%d", _localauth_enable);
        set_param_value(SQLITE_FILE_NAME, PARAM_GUI_AUTH, tmp);
        wantLogWrite = 1;
    }
    if (_menuauth_enable != ui->comboBox_menuAuth->currentIndex()) {
        _menuauth_enable = ui->comboBox_menuAuth->currentIndex();
        char tmp[20] = { 0 };
        snprintf(tmp, sizeof(tmp), "%d", _menuauth_enable);
        set_param_value(SQLITE_FILE_NAME, PARAM_GUI_MENU_AUTH, tmp);
        wantLogWrite = 1;
    }
    if (_menu_timeout != ui->comboBox_menuTimeout->itemData(ui->comboBox_menuTimeout->currentIndex()).value<int>()) {
        _menu_timeout = ui->comboBox_menuTimeout->itemData(ui->comboBox_menuTimeout->currentIndex()).value<int>();
        char tmp[20] = { 0 };
        snprintf(tmp, sizeof(tmp), "%d", _menu_timeout);
        set_param_value(SQLITE_FILE_NAME, PARAM_GUI_MENU_TIME_OUT, tmp);
        emit sigSetTimeOut(_menu_timeout);
        SettingContent::instance()->initializeSettingTimeout();
        wantLogWrite = 1;
    }

    int languageFlag = 0;
    int languageReboot = 0;
    int language_id = ui->comboBox_language->currentData().toInt();

    //if(sys_language != 1)//1: 中文
    languageFlag = _sys_language != language_id ? 1 : 0;

    //
    bool homologousRboot = false;
    if (ui->comboBox_hdmi_vga_output->isVisible()) {
        int homologous = ui->comboBox_hdmi_vga_output->currentIndex();
        qDebug() << "----previous homologous:" << m_homologous;
        qDebug() << "----current homologous:" << homologous;
        if (homologous != m_homologous) {
            if (MessageBox::Yes == MessageBox::question(this, GET_TEXT("SYSTEMGENERAL/70059", "The modification will take effect after rebooting. Are you sure to reboot now?"))) {
                if (homologous == 0) {
                    pDisplayDb->sub_enable = 1;
                } else {
                    pDisplayDb->sub_enable = 0;
                }
                //qMsNvr->writeDisplayInfo(pDisplayDb);

                //
                set_param_int(SQLITE_FILE_NAME, PARAM_HOMOLOGOUS, homologous);
                homologousRboot = true;
                wantLogWrite = 1;
            }
        }
    }
    //
    if (languageFlag == 1) {
        qDebug() << "old language id:" << old_language << " new:" << language_id << " 16&17&19 need reboot";
        if (language_id == LNG_ARABIC || language_id == LNG_HEBREW || language_id == LGN_PERSIAN) {
            if (old_language != LNG_ARABIC && old_language != LNG_HEBREW && old_language != LGN_PERSIAN) {
                languageReboot = 1;
            } else if (language_id != old_language) {
                languageReboot = 1;
            }
        }

        if (languageReboot) {
            if (MessageBox::Yes == MessageBox::question(this, "Language changes will take effect after rebooting. Are you sure to reboot now?")) {
                saveLanguage();
                wantLogWrite = 1;
            } else {
                languageReboot = 0;
            }
        } else {
            saveLanguage();
            wantLogWrite = 1;
        }
    }

    //
    //qMsNvr->setQuickSwitchScreenEnable(ui->comboBox_screenSwitch->currentIndex());

    //
    gAutoLogout.writeInterval(ui->comboBoxAutoLogout->currentData().toInt());

    //
    if (wantLogWrite) {
        struct log_data log_data;
        memset(&log_data, 0, sizeof(struct log_data));
        log_data.log_data_info.subType = SUB_OP_CONFIG_LOCAL;
        log_data.log_data_info.parameter_type = SUB_PARAM_SYSTEM_GENERAL;
        snprintf(log_data.log_data_info.user, sizeof(log_data.log_data_info.user), "%s", gMsUser.userName().toStdString().c_str());
        log_data.log_data_info.chan_no = 0;
        MsWriteLog(log_data);
    }

    if (languageReboot || homologousRboot) {
        //qMsNvr->reboot();
        //qMsApp->setAboutToReboot(true);
    }

    // 临时解决切换分辨率后某些界面无法获取焦点导致不能弹出软键盘的问题
    //MsWaitting::showGlobalWait();
    //MsWaitting::closeGlobalWait();
}

void GeneralSetting::on_pushButton_back_clicked()
{
    emit sig_back();
}

void GeneralSetting::on_pushButton_wizard_clicked()
{
    SettingContent::instance()->closeSetting();
    MainWindow::s_mainWindow->showWizard();
}

void GeneralSetting::on_pushButton_back_2_clicked()
{
    emit sig_back();
}

void GeneralSetting::on_pushButton_apply_2_clicked()
{
    bool valid = true;
    if (ui->comboBox_timeSetting->currentIndex() == AUTO_TIME) {
        valid &= ui->lineEdit_ntpServer->checkValid();
    }
    if (ui->comboBox_ntpSync->currentIntData()) {
        valid &= ui->lineEdit_interval->checkValid();
    }
    if (!valid) {
        return;
    }

    //【ID1044397】中心-Auto Backup：修改时间或时区，会导致备份终止，且不续传，需要兼容项比较多；建议修改时间或时区时，告知客户会停止备份【提供提示语，含自动备份&后台导出】
    if (DownloadPanel::instance() && DownloadPanel::instance()->isDownloading()) {
        const int result = MessageBox::question(this, GET_TEXT("AUTOBACKUP/95218", "Current backup will be affected if settings is changed, continue?"));
        if (result == MessageBox::Cancel) {
            return;
        }
    }

    //showWait();
    sendMessage(REQUEST_FLAG_GET_ESATA_BACKUP_STATUS, nullptr, 0);
    // if (m_eventLoop.exec() == ESATA_BACKUP_WORKING) {
    //     //closeWait();
    //     const int result = MessageBox::question(this, GET_TEXT("AUTOBACKUP/95218", "Current backup will be affected if settings is changed, continue?"));
    //     if (result == MessageBox::Cancel) {
    //         return;
    //     }
    // }

    //
    int sncTimeLog = 0;
    struct req_set_sysconf timeconf;

    old_zone_index = ui->comboBox_timezone->currentIndex();
    QString str = ui->comboBox_timezone->itemData(old_zone_index, Qt::UserRole).value<QString>();
    sscanf(str.toStdString().c_str(), "%31s %31s", pTimeDb->time_zone, pTimeDb->time_zone_name);
    pTimeDb->dst_enable = ui->comboBox_dsTime->currentIndex();
    if (ui->comboBox_timeSetting->currentIndex() == AUTO_TIME) {
        pTimeDb->ntp_enable = 1;
    } else {
        pTimeDb->ntp_enable = 0;
    }
    pTimeDb->sync_pc = 0;
    snprintf(pTimeDb->ntp_server, sizeof(pTimeDb->ntp_server), "%s", ui->lineEdit_ntpServer->text().trimmed().toStdString().c_str());

    pTimeDb->sync_enable = ui->comboBox_ntpSync->currentIndex();
    pTimeDb->sync_interval = ui->lineEdit_interval->text().toInt();

    memset(&timeconf, 0, sizeof(timeconf));
    memcpy(pTimeDbOri, pTimeDb, sizeof(struct time));
    write_time(SQLITE_FILE_NAME, pTimeDb);
    if (!qMsNvr->isSlaveMode()) {
        QDateTime datetime = ui->dateTimeEdit_setTime->dateTime();
        snprintf(timeconf.arg, sizeof(timeconf.arg), "\"%s\"", datetime.toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());
    }
    on_dateTimeEdit_setTime_dateTimeChanged(ui->dateTimeEdit_setTime->dateTime());

    sendMessage(REQUEST_FLAG_SET_SYSTIME, (void *)&timeconf, sizeof(timeconf));
    sncTimeLog = 1;
    if (sncTimeLog == 1) {
        struct log_data log_data;
        memset(&log_data, 0, sizeof(struct log_data));
        log_data.log_data_info.subType = SUP_OP_SYSTEM_TIME_SYNC_LOCAL;
        log_data.log_data_info.parameter_type = SUB_PARAM_NONE;
        snprintf(log_data.log_data_info.user, sizeof(log_data.log_data_info.user), "%s", gMsUser.userName().toStdString().c_str());
        log_data.log_data_info.chan_no = 0;
        MsWriteLog(log_data);
    }
}

void GeneralSetting::on_comboBox_ntpSync_activated(int index)
{
    if (index == 0) {
        ui->lineEdit_interval->setDisabled(true);
    } else {
        ui->lineEdit_interval->setDisabled(false);
    }
}

int GeneralSetting::ntpIntervalTransToIndex(int value)
{
    int interval = 0;
    switch (value) {
    case 1 * 60:
        interval = 0;
        break;
    case 2 * 60:
        interval = 1;
        break;
    case 5 * 60:
        interval = 2;
        break;
    case 10 * 60:
        interval = 3;
        break;
    case 24 * 60:
        interval = 4;
        break;
    case 2 * 24 * 60:
        interval = 5;
        break;
    case 5 * 24 * 60:
        interval = 6;
        break;
    case 10 * 24 * 60:
        interval = 7;
        break;
    case 30 * 24 * 60:
        interval = 8;
        break;

    default:
        interval = 4;
        break;
    }

    return interval;
}
int GeneralSetting::ntpIntervalTransToMinutes(int value)
{
    int interval = 0;
    switch (value) {
    case 0:
        interval = 60;
        break;
    case 1:
        interval = 2 * 60;
        break;
    case 2:
        interval = 5 * 60;
        break;
    case 3:
        interval = 10 * 60;
        break;
    case 4:
        interval = 24 * 60;
        break;
    case 5:
        interval = 2 * 24 * 60;
        break;
    case 6:
        interval = 5 * 24 * 60;
        break;
    case 7:
        interval = 10 * 24 * 60;
        break;
    case 8:
        interval = 30 * 24 * 60;
        break;
    default:
        interval = 24 * 60;
        break;
    }

    return interval;
}

void GeneralSetting::on_comboBox_timezone_activated(int index)
{
    Q_UNUSED(index)
    int year = 0, month = 0, day = 0;
    int min = 0, hour = 0, sec = 0;


    QDate d(year, month, day);
    QTime t(hour, min, sec);
    QDateTime n(d, t);
    time_zone_changed = 1;
    ui->dateTimeEdit_setTime->setDateTime(n);
}

int GeneralSetting::getTimeZoneOffsetSec(int index)
{
    char timeZone[32] = { 0 };
    char timeZoneName[32] = { 0 };
    int offset = 0;
    int hour = 0;
    int min = 0;

    QString str = ui->comboBox_timezone->itemData(index, Qt::UserRole).value<QString>();
    sscanf(str.toStdString().c_str(), "%31s %31s", timeZone, timeZoneName);
    sscanf(timeZone, "UTC%d:%d", &hour, &min);

    if (hour < 0) {
        offset = -hour * 60 * 60 + min * 60;
    } else {
        offset = -hour * 60 * 60 - min * 60;
    }

    return offset;
}

void GeneralSetting::on_dateTimeEdit_setTime_dateTimeChanged(const QDateTime &dateTime)
{
    if (time_zone_changed == 1) {
        time_zone_changed = 0;
    } else {
        snprintf(init_time, sizeof(init_time), "%s", dateTime.toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());
    }

    return;
}

void GeneralSetting::on_comboBox_screenSwitch_activated(int index)
{
    if (index == 1) {
        ShowMessageBox(GET_TEXT("SYSTEMGENERAL/70029", "Double-click the mouse wheel to switch Main/Sub screen control!"));
    }
}

void GeneralSetting::on_comboBox_mouseAccel_activated(int index)
{
    Q_UNUSED(index)

    qreal accel = ui->comboBox_mouseAccel->currentData().toDouble();
    ScreenController::instance()->setMouseSpeed(accel);

    set_param_value(SQLITE_FILE_NAME, "mouse_accel", QString::number(accel).toStdString().c_str());
}

void GeneralSetting::on_pushButtonLogoutChannel_clicked()
{
    ui->pushButtonLogoutChannel->clearUnderMouse();

    LogoutChannel::instance()->showLogoutChannel();
}

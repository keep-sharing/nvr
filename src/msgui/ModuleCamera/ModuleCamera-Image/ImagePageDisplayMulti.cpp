#include "ImagePageDisplayMulti.h"
#include "ui_ImagePageDisplayMulti.h"
#include "FaceData.h"
#include "MessageBox.h"
#include "MsCameraVersion.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MsSdkVersion.h"
#include "MsWaitting.h"
#include "MyDebug.h"
#include "centralmessage.h"
#include "channelcopydialog.h"
#include <QListView>
#include <QScopedValueRollback>
#include <QtCore/qmath.h>
#include <cmath>
#include "qjson/include/parser.h"

ImagePageDisplayMulti::ImagePageDisplayMulti(QWidget *parent)
    : AbstractImagePage(parent)
      , ui(new Ui::ImagePageDisplayMulti)
{
  ui->setupUi(this);

  ui->comboBox_PowerLineFrequency->clear();
  ui->comboBox_PowerLineFrequency->addItem("60Hz");
  ui->comboBox_PowerLineFrequency->addItem("50Hz");

  ui->comboBox_SmartIrMode->clear();
  ui->comboBox_SmartIrMode->addItem(GET_TEXT("COMMON/1015", "Customize"), 1);

  ui->comboBox_DayNightMode->clear();
  ui->comboBox_DayNightMode->addItem(GET_TEXT("IMAGE/37103", "Night Mode"), 1);
  ui->comboBox_DayNightMode->addItem(GET_TEXT("IMAGE/37102", "Day Mode"), 2);
  ui->comboBox_DayNightMode->addItem(GET_TEXT("IMAGE/37104", "Auto Mode"), 0);
  ui->comboBox_DayNightMode->addItem(GET_TEXT("IMAGE/37105", "Customize"), 3);


  ui->comboBox_ImageRotation->clear();
  ui->comboBox_ImageRotation->addItem(GET_TEXT("IMAGE/37316", "Off"));
  ui->comboBox_ImageRotation->addItem(GET_TEXT("IMAGE/37122", "Rotating 180°"));

  ui->slider_DayToNightSensitivity->setRange(1, 10);
  ui->slider_NightToDaySensitivity->setRange(1, 10);

  connect(ui->comboBox_DayNightMode, SIGNAL(currentIndexChanged(int)), this, SLOT(onDayNightModeChange(int)));
  connect(ui->slider_DayToNightSensitivity, SIGNAL(valueChanged(int)), this, SLOT(onDSensitivityChanged(int)));
  connect(ui->slider_NightToDaySensitivity, SIGNAL(valueChanged(int)), this, SLOT(onNSensitivityChanged(int)));
  connect(ui->timeEdit_StartTimeOfNight, SIGNAL(timeChanged(QTime)), this, SLOT(onDayNightTimeChange(QTime)));
  connect(ui->timeEdit_EndTimeOfNight, SIGNAL(timeChanged(QTime)), this, SLOT(onDayNightTimeChange(QTime)));

  onLanguageChanged();
}

ImagePageDisplayMulti::~ImagePageDisplayMulti()
{
  delete ui;
}

void ImagePageDisplayMulti::ON_RESPONSE_FLAG_GET_SINGLE_IPCTYPE(MessageReceive *message)
{
  CAM_MODEL_INFO *cam_model_info = (CAM_MODEL_INFO *)message->data;
  if (!cam_model_info) {
    m_eventLoop.exit(-1);
    return;
  }
  m_model.setModel(cam_model_info->model);
  m_eventLoop.exit();
}

void ImagePageDisplayMulti::initializeData(int channel)
{
  msprintf("gsjt ImagePageDisplayMulti initializeData");
  m_channel = channel;

  m_isInitDisplay = true;
  clearSettings();
  setSettingsEnable(false);

  if (!isChannelConnected()) {
    ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20102", "This channel is not connected."));
    return;
  }
  if (!qMsNvr->isMsCamera(m_channel)) {
    ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20107", "This channel does not support this function."));
    return;
  }

  setSettingsEnable(true);
  setNeedDNSensitivity(true);

  //MsWaitting::showGlobalWait();
  sendMessage(REQUEST_FLAG_GET_SINGLE_IPCTYPE, &m_channel, sizeof(int));
  //m_eventLoop.exec();
  sendMessage(REQUEST_FLAG_GET_IPCIMAGE_DISPLAY, &m_channel, sizeof(int));
  //m_eventLoop.exec();
  sendMessage(REQUEST_FLAG_GET_IPC_MODEL_TYPE, &m_channel, sizeof(int));
  //m_eventLoop.exec();
  sendMessage(REQUEST_FLAG_GET_LPR_SETTINGS, &m_channel, sizeof(int));
  //m_eventLoop.exec();
  //MsWaitting::closeGlobalWait();
}

void ImagePageDisplayMulti::processMessage(MessageReceive *message)
{
  switch (message->type()) {
  case RESPONSE_FLAG_GET_SINGLE_IPCTYPE:
    ON_RESPONSE_FLAG_GET_SINGLE_IPCTYPE(message);
    break;
  case RESPONSE_FLAG_GET_IPCIMAGE_DISPLAY:
    ON_RESPONSE_FLAG_GET_IPCIMAGE_DISPLAY(message);
    break;
  case RESPONSE_FLAG_GET_IPC_MODEL_TYPE:
    ON_RESPONSE_FLAG_GET_IPC_MODEL_TYPE(message);
    break;
  case RESPONSE_FLAG_GET_IPC_LPR_NIGHT_MODE:
    ON_RESPONSE_FLAG_GET_IPC_LPR_NIGHT_MODE(message);
    break;
  case RESPONSE_FLAG_GET_LPR_SETTINGS:
    ON_RESPONSE_FLAG_GET_LPR_SETTINGS(message);
    break;
  default:
    break;
  }
}

int ImagePageDisplayMulti::defaultNearViewIRValue(const MsCameraModel &model)
{
  int value = 50;
  if (model.isMatch(QRegExp("MS-C..75-EPB"))) {
    value = 100;
  } else if (model.isMatch(QRegExp("MS-C9674-PB"))) {
    value = 80;
  } else if (model.isMatch(QRegExp("MS-C5374-PB"))) {
    value = 80;
  }
  return value;
}

int ImagePageDisplayMulti::defaultMiddleViewIRValue(const MsCameraModel &model)
{
  Q_UNUSED(model)

  int value = 50;
  return value;
}

int ImagePageDisplayMulti::defaultFarViewIRValue(const MsCameraModel &model)
{
  int value = 50;
  if (model.isMatch(QRegExp("MS-C9674-PB"))) {
    value = 60;
  } else if (model.isMatch(QRegExp("MS-C5374-PB"))) {
    value = 60;
  } else if (model.isMatch(QRegExp("MS-C..75-F.*"))) {
    value = 100;
  }
  return value;
}

int ImagePageDisplayMulti::defaultIRLEDLevel(const MsCameraModel &model)
{
  Q_UNUSED(model)

  int value = 100;
  return value;
}

void ImagePageDisplayMulti::ON_RESPONSE_FLAG_GET_IPC_LPR_NIGHT_MODE(MessageReceive *message)
{
  resp_lpr_night_mode *data = reinterpret_cast<resp_lpr_night_mode *>(message->data);
  if (!data) {
    qWarning() << __FUNCTION__ << ": data is null";
    m_eventLoop.exit(-1);
    qDebug() << "@@@@@ failed isNightMode:" << isNightMode;
    return;
  }
  isNightMode = data->nightMode;
  qDebug() << "######ON_RESPONSE_FLAG_GET_IPC_LPR_NIGHT_MODE isNightMode:" << isNightMode;
  m_eventLoop.exit();
}

void ImagePageDisplayMulti::ON_RESPONSE_FLAG_GET_LPR_SETTINGS(MessageReceive *message)
{
  struct ms_lpr_settings *lpr_settings = (struct ms_lpr_settings *)message->data;
  if (!lpr_settings) {
    qWarning() << QString("ImagePageDisplayMulti::ON_RESPONSE_FLAG_GET_LPR_SETTINGS, data is null. isNightMode:[%1]").arg(isNightMode);
    m_eventLoop.exit();
    return;
  }
  isNightMode = lpr_settings->in_night_mode;
  m_nightModeEnable = lpr_settings->night_mode_info.enable;
  m_nightModeEffectiveMode = lpr_settings->night_mode_info.effective_mode;
  qDebug() << "###### ON_RESPONSE_FLAG_GET_LPR_SETTINGS  isNightMode:" << isNightMode;
  m_eventLoop.exit();
}

void ImagePageDisplayMulti::ON_RESPONSE_FLAG_GET_IPCIMAGE_DISPLAY(MessageReceive *message)
{
  resp_image_display *data = (resp_image_display *)message->data;
  if (!data) {
    qWarning() << "ImagePageDisplayMulti::ON_RESPONSE_FLAG_GET_IPCIMAGE_DISPLAY, data is null.";
    m_eventLoop.exit(-1);
    return;
  }
  memcpy(&m_info, &(data->image), sizeof(IMAGE_DISPLAY_MULTI_S));
  initDisplay();

  m_eventLoop.exit();
}

void ImagePageDisplayMulti::ON_RESPONSE_FLAG_SET_IPCIMAGE_DISPLAY(MessageReceive *message)
{
  Q_UNUSED(message)
  if (m_copyChannelList.isEmpty()) {
    //MsWaitting::closeGlobalWait();
  }
}

void ImagePageDisplayMulti::ON_RESPONSE_FLAG_GET_IPC_MODEL_TYPE(MessageReceive *message)
{
  if (!message->data) {
    qWarning() << "ImagePageDisplayMulti::ON_RESPONSE_FLAG_GET_IPC_MODEL_TYPE, data is null.";
    m_eventLoop.exit(-1);
    return;
  }

  int type = *((int *)message->data);
  m_currentChnType = type;

  //IPC需为73版本或者以上才支持该功能，低于73版本的IPC不显示该项
  //除去鱼眼+全景筒，其他所有机型都支持该功能
  char version[50];
  //get_channel_version_name(m_channel, version, sizeof(version));
  MsCameraVersion cameraVersion(version);

  setNeedZoomLimit(false);
  setNeedDNSensitivity(false);
  if (cameraVersion >= MsCameraVersion(7, 74)) {
    if (m_model.opticalZoom() > 20) {
      setNeedZoomLimit(true);
    }
    setNeedDNSensitivity(true);
  }

  updateCorridor(type);
  m_eventLoop.exit();
}

bool ImagePageDisplayMulti::checkCorridor(int type)
{
  bool supportCorridor = false;
  char version[50];
  //get_channel_version_name(m_channel, version, sizeof(version));
  MsCameraVersion cameraVersion(version);
  if (type == AMBA_IPC) {
    if (cameraVersion >= MsCameraVersion(7, 65) && !qMsNvr->isFisheye(m_channel)) {
      supportCorridor = true;
    }
  } else {
    if (cameraVersion >= MsCameraVersion(7, 60)) {
      supportCorridor = true;
    }
  }
  return supportCorridor;
}

void ImagePageDisplayMulti::updateCorridor(int type)
{
  m_supportCorridor = checkCorridor(type);
  if (m_supportCorridor) {

    ui->label_ImageRotation->show();
    ui->comboBox_ImageRotation->show();
  } else {

    ui->label_ImageRotation->hide();
    ui->comboBox_ImageRotation->hide();
  }
}

void ImagePageDisplayMulti::initDisplay()
{
  ui->comboBox_PowerLineFrequency->setCurrentIndex(m_info.scenes[0].powerlineFreq);

  ui->comboBox_DayNightMode->setCurrentIndexFromData(m_info.scenes[0].dnMode);

  ui->slider_DayToNightSensitivity->setValue(m_info.scenes[0].d2nSensitivity);
  ui->slider_NightToDaySensitivity->setValue(m_info.scenes[0].n2dSensitivity);
  ui->timeEdit_StartTimeOfNight->setTime(QTime(m_info.scenes[0].nightStartHour, m_info.scenes[0].nightStartMinute));
  ui->timeEdit_EndTimeOfNight->setTime(QTime(m_info.scenes[0].nightEndHour, m_info.scenes[0].nightEndMinute));

  ui->comboBox_ImageRotation->setCurrentIndex(m_info.scenes[0].imageRotation);

  ui->slider_IRLedValue->setValue(m_info.scenes[0].irLedLevel);

  ui->label_SensitivitySanityNote->hide();

  onDayNightModeChange(ui->comboBox_DayNightMode->currentIndex());

  m_isInitDisplay = false;
}

void ImagePageDisplayMulti::onDSensitivityChanged(int)
{
  sensitivitySanityCheck(true);
}

void ImagePageDisplayMulti::onNSensitivityChanged(int)
{
  sensitivitySanityCheck(false);
}

void ImagePageDisplayMulti::sensitivitySanityCheck(bool DPreferred)
{
  if (inAdjustSensitivity)
    return;

  bool showSanityNote = false;

  if (ui->slider_DayToNightSensitivity->value() + ui->slider_NightToDaySensitivity->value() <= dnSensitivityLimit) {
    ui->label_SensitivitySanityNote->setVisible(showSanityNote);
    return;
  }

  showSanityNote = true;
  ui->label_SensitivitySanityNote->setVisible(showSanityNote);
  m_daySensitivityPreferred = DPreferred;
  adjustSensitivity();
}

void ImagePageDisplayMulti::adjustSensitivity()
{
  QScopedValueRollback<bool> cleanup(inAdjustSensitivity);
  inAdjustSensitivity = true;

  int dSensitivity = ui->slider_DayToNightSensitivity->value();
  int nSensitivity = ui->slider_NightToDaySensitivity->value();

  if (m_daySensitivityPreferred)
    nSensitivity = dnSensitivityLimit - dSensitivity;
  else
    dSensitivity = dnSensitivityLimit - nSensitivity;

  ui->slider_DayToNightSensitivity->setValue(dSensitivity);
  ui->slider_NightToDaySensitivity->setValue(nSensitivity);
  ui->label_SensitivitySanityNote->setVisible(true);
}

void ImagePageDisplayMulti::mayDisplayDNSensitivityPanel(bool visible)
{
  msprintf("gsjt %d",needDNSensitivity());
  ui->label_DayToNightSensitivity->setVisible(visible && needDNSensitivity());
  ui->widget_DayToNightSensitivity->setVisible(visible && needDNSensitivity());
  ui->label_NightToDaySensitivity->setVisible(visible && needDNSensitivity());
  ui->widget_NightToDaySensitivity->setVisible(visible && needDNSensitivity());
}



bool ImagePageDisplayMulti::needDNSensitivity()
{
  return m_needDNSensitivity;
}

void ImagePageDisplayMulti::setNeedDNSensitivity(bool b)
{
  if (m_needDNSensitivity == b)
    return;
  m_needDNSensitivity = b;
  emit needDNSensitivityChanged();
}

bool ImagePageDisplayMulti::needZoomLimit()
{
  return m_needZoomLimit;
}

void ImagePageDisplayMulti::setNeedZoomLimit(bool b)
{
  if (m_needZoomLimit == b) {
    return;
  }
  m_needZoomLimit = b;
  emit needZoomLimitChanged();
}

bool ImagePageDisplayMulti::isAFLens3X()
{
  struct ipc_system_info system_info;
  memset(&system_info, 0, sizeof(system_info));
  //get_ipc_system_info(m_channel, &system_info);
  if (system_info.chipinfo[16] == '6') {
    return true;
  }
  return false;
}

bool ImagePageDisplayMulti::isSpeedLens36Or42()
{
  struct ipc_system_info system_info;
  memset(&system_info, 0, sizeof(system_info));
  //get_ipc_system_info(m_channel, &system_info);
  if (system_info.chipinfo[16] == '7' || system_info.chipinfo[16] == '8') {
    return true;
  }
  return false;
}

bool ImagePageDisplayMulti::isHisiIPC()
{
  char version[50];
  //get_channel_version_name(m_channel, version, sizeof(version));
  MsCameraVersion cameraVersion(version);
  if (cameraVersion.chipType() == 4 && (cameraVersion.chipVersion() == 3 || cameraVersion.chipVersion() == 5)) {
    return true;
  }
  return false;
}

bool ImagePageDisplayMulti::isPoe(char *productmodel)
{
  QString model = QString("%1").arg(productmodel);

  if (model.contains('P')) {
    return true;
  }
  return false;
}

bool ImagePageDisplayMulti::isSupportDayNightSwitchRefocus()
{
  //AF镜头 或变焦镜头7~22mm的机型支持。
  //加密芯片第17位为S\W\Y\Z\a\f\g\5\l\J
  QList<char> list;
  list << 'S' << 'W' << 'Y' << 'Z' << 'a' << 'f' << 'g' << '5' << 'l' << 'J' << 'V';

  struct ipc_system_info system_info;
  memset(&system_info, 0, sizeof(system_info));
  //get_ipc_system_info(m_channel, &system_info);
  if (list.contains(system_info.chipinfo[16])) {
    return true;
  }
  return false;
}


void ImagePageDisplayMulti::onNeedDNSensitivityChanged()
{
  onDayNightModeChange(ui->comboBox_DayNightMode->currentIndex());
}

void ImagePageDisplayMulti::onDayNightModeChange(int index)
{
  mayDisplayDNSensitivityPanel(false);

  ui->label_StartTimeOfNight->hide();
  ui->timeEdit_StartTimeOfNight->hide();
  ui->label_EndTimeOfNight->hide();
  ui->timeEdit_EndTimeOfNight->hide();
  msprintf("gsjt onDayNightModeChange:%d",index);
  if (index == 2) {
    mayDisplayDNSensitivityPanel(true);
  } else if (index == 3) {
    ui->label_StartTimeOfNight->show();
    ui->timeEdit_StartTimeOfNight->show();
    ui->label_EndTimeOfNight->show();
    ui->timeEdit_EndTimeOfNight->show();
  }
}

void ImagePageDisplayMulti::onLanguageChanged()
{
  ui->label_PowerLineFrequency->setText(GET_TEXT("IMAGE/37100", "Power Line Frequency"));
  ui->label_SmartIrMode->setText(GET_TEXT("IMAGE/37109", "Smart IR Mode"));

  ui->label_DayNightMode->setText(GET_TEXT("IMAGE/37101", "Day/Night Mode"));
  ui->label_DayToNightSensitivity->setText(GET_TEXT("IMAGE/37411", "Day to Night Sensitivity"));
  ui->label_NightToDaySensitivity->setText(GET_TEXT("IMAGE/37412", "Night to Day Sensitivity"));
  ui->label_SensitivitySanityNote->setText(GET_TEXT("IMAGE/37413", "Note: The sum of sensitivity should be within 15."));

  ui->label_StartTimeOfNight->setText(GET_TEXT("IMAGE/37127", "Start time of Night"));
  ui->label_EndTimeOfNight->setText(GET_TEXT("IMAGE/37128", "End time of Night"));

  ui->label_ImageRotation->setText(GET_TEXT("IMAGE/37120", "Image Rotation"));

  ui->pushButton_IRLedValue->setText(GET_TEXT("COMMON/1057", "Reset"));

  ui->pushButton_DayToNightSensitivity->setText(GET_TEXT("COMMON/1057", "Reset"));
  ui->pushButton_NightToDaySensitivity->setText(GET_TEXT("COMMON/1057", "Reset"));

  ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));
  ui->pushButton_apply->setText(GET_TEXT("COMMON/1003", "Apply"));
  ui->pushButton_copy->setText(GET_TEXT("COMMON/1005", "Copy"));

}

void ImagePageDisplayMulti::clearSettings()
{
  ui->widgetMessage->hideMessage();
  //初始化UI
  ui->pushButton_test->hide();
  ui->comboBox_SmartIrMode->setCurrentIndex(0);
  ui->comboBox_DayNightMode->setCurrentIndex(0);
  ui->comboBox_PowerLineFrequency->setCurrentIndex(0);

  ui->comboBox_ImageRotation->setCurrentIndex(0);

  onDayNightModeChange(0);
}

void ImagePageDisplayMulti::setSettingsEnable(bool enable)
{
  ui->scrollArea->setEnabled(enable);

  ui->pushButton_copy->setEnabled(enable);
  ui->pushButton_apply->setEnabled(enable);
}

int ImagePageDisplayMulti::whiteandIRPWMInterface(int m_chipninterface)
{
  if (m_chipninterface & m_PWM_1IR_1W)
    return PWM_TYPE_1IR_1W;
  else if (m_chipninterface & m_PWM_2IR)
    return PWM_TYPE_2IR;
  else if (m_chipninterface & m_PWM_2IR_1W)
    return PWM_TYPE_2IR_1W;
  else if (m_chipninterface & m_PWM_2IR_1NW)
    return PWM_TYPE_2IR_1NW;
  else if (m_chipninterface & m_PWM_1IR)
    return PWM_TYPE_1IR;
  else if (m_chipninterface & m_PWM_3IR)
    return PWM_TYPE_3IR;
  else if (m_chipninterface & m_PWM_1IR_2W)
    return PWM_TYPE_1IR_2W;
  else
    return PWM_TYPE_NONE;
}

bool ImagePageDisplayMulti::isLessOldPWMInterface(int m_chipninterface)
{
  bool value;
  if ((m_chipninterface & m_pwm) != 0 || (m_chipninterface & m_pwm_2) != 0)
    value = false;
  else
    value = true;

  return value;
}

bool ImagePageDisplayMulti::isSupDistanceMeasure(int m_chipninterface)
{
  if ((m_chipninterface & m_distance_measure) != 0)
    return true;

  return false;
}

bool ImagePageDisplayMulti::isFixFocus(int m_chipninterface)
{
  if ((m_chipninterface & m_fix_len) != 0)
    return true;

  return false;
}

bool ImagePageDisplayMulti::isLessBncInterface(int m_chipninterface)
{
  if ((m_chipninterface & m_bnc_len) != 0)
    return false;

  return true;
}

bool ImagePageDisplayMulti::isProDome(char *productmodel)
{
  QString model = QString("%1").arg(productmodel);
  QString sMini = model.mid(6, 2);
  if (sMini == "72")
    return true;

  return false;
}

bool ImagePageDisplayMulti::isSpeed(char *productmodel)
{
  QString model = QString("%1").arg(productmodel);
  int sMini = model.mid(6, 1).toInt();
  if (sMini == 4)
    return true;

  return false;
}

bool ImagePageDisplayMulti::isMiniZoomDome(char *productmodel)
{
  QString model = QString("%1").arg(productmodel);
  QString sNewIRmini = model.mid(6, 2);
  if (sNewIRmini == "75")
    return true;

  return false;
}

bool ImagePageDisplayMulti::isSmokedDomeCoverSensor(char *sensortype)
{
  QString sensor = QString("%1").arg(sensortype);
  if (sensor.contains("imx385") || sensor.contains("imx290")
      || sensor.contains("imx291") || sensor.contains("imx123"))
    return true;
  else
    return false;
}
bool ImagePageDisplayMulti::isMiniPtzDome(char *productmodel)
{
  QString model = QString("%1").arg(productmodel);
  int sBullet1 = model.mid(6, 1).toInt();
  int sBullet2 = model.mid(7, 1).toInt();
  if (sBullet1 == 7 && sBullet2 == 1)
    return true;
  return false;
}

bool ImagePageDisplayMulti::isMiniWithoutIR(char *productmodel)
{
  QString model = QString("%1").arg(productmodel);
  QString sMini = model.mid(6, 2);
  struct ipc_system_info system_info;
  memset(&system_info, 0, sizeof(system_info));
  //get_ipc_system_info(m_channel, &system_info);
  if (sMini == "63" && system_info.chipinfo[13] == '2') {
    return true;
  }
  return false;
}

bool ImagePageDisplayMulti::isBox(char *productmodel)
{
  QString model = QString("%1").arg(productmodel);
  int tmp = model.mid(6, 1).toInt();
  if (tmp == 5)
    return true;
  return false;
}

bool ImagePageDisplayMulti::isMiniProBulletFull(char *productmodel)
{
  QString model = QString("%1").arg(productmodel);
  int mini1 = model.mid(6, 1).toInt();
  int mini2 = model.mid(7, 1).toInt();
  if (mini1 == 6 && mini2 == 5) {
    return true;
  } else if (mini1 == 7 && mini2 == 6) {
    return true;
  }
  return false;
}

int ImagePageDisplayMulti::saveDisplayInfo()
{
  struct req_image_display data;
  memset(&data, 0, sizeof(struct req_image_display));
  data.chnid = m_copyChannelList.takeFirst();
  data.type = IPC_IMAGE_TYPE_MULTI;

  data.imgMulti.scenes[0].powerlineFreq = (IPC_POWERLINE_FREQ_E)ui->comboBox_PowerLineFrequency->currentIndex();
  data.imgMulti.scenes[0].smartIrMode = (IPC_SMARTIR_MODE_E)ui->comboBox_SmartIrMode->currentData().toInt();


  data.imgMulti.scenes[0].irLedLevel = ui->slider_IRLedValue->value();


  data.imgMulti.scenes[0].dnMode = (IPC_DN_MODE_E)ui->comboBox_DayNightMode->currentIntData();

  data.imgMulti.scenes[0].d2nSensitivity = ui->slider_DayToNightSensitivity->value();
  data.imgMulti.scenes[0].n2dSensitivity = ui->slider_NightToDaySensitivity->value();

  QTime nightStart = ui->timeEdit_StartTimeOfNight->time();
  QTime nightEnd = ui->timeEdit_EndTimeOfNight->time();
  data.imgMulti.scenes[0].nightStartHour = nightStart.hour();
  data.imgMulti.scenes[0].nightStartMinute = nightStart.minute();
  data.imgMulti.scenes[0].nightEndHour = nightEnd.hour();
  data.imgMulti.scenes[0].nightEndMinute = nightEnd.minute();
  data.imgMulti.scenes[0].imageRotation = (IPC_IMAGE_ROTATION_E)ui->comboBox_ImageRotation->currentIndex();

  sendMessage(REQUEST_FLAG_SET_IPCIMAGE_DISPLAY, &data, sizeof(struct resp_image_display));

  return 0;
}

void ImagePageDisplayMulti::on_pushButton_back_clicked()
{
  back();
}

void ImagePageDisplayMulti::on_pushButton_apply_clicked()
{
  //MsWaitting::showGlobalWait();

  if (m_copyChannelList.isEmpty()) {
    m_copyChannelList.append(m_channel);
  }

  m_isCopy = m_copyChannelList.size() > 1;
  do {
    int result = saveDisplayInfo();
    if (result < 0) {
      //MsWaitting::closeGlobalWait();
      return;
    }

    qApp->processEvents();
  } while (!m_copyChannelList.isEmpty());
  QTimer::singleShot(1000, this, SLOT(onUpdatePlayVideo()));
  sendMessage(REQUEST_FLAG_GET_IPCIMAGE_DISPLAY, &m_channel, sizeof(int));
  //m_eventLoop.exec();
  //MsWaitting::closeGlobalWait();
}

void ImagePageDisplayMulti::on_pushButton_copy_clicked()
{
  m_copyChannelList.clear();

  ChannelCopyDialog copy(this);
  copy.setCurrentChannel(m_channel);
  int result = copy.exec();
  if (result == QDialog::Accepted) {
    m_copyChannelList = copy.checkedList();
    QTimer::singleShot(0, this, SLOT(on_pushButton_apply_clicked()));
  }
}

void ImagePageDisplayMulti::on_pushButton_DayToNightSensitivity_clicked()
{
  ui->slider_DayToNightSensitivity->setValue(5);
}

void ImagePageDisplayMulti::on_pushButton_NightToDaySensitivity_clicked()
{
  ui->slider_NightToDaySensitivity->setValue(5);
}

void ImagePageDisplayMulti::on_pushButton_IRLedValue_clicked()
{
    ui->slider_IRLedValue->setValue(defaultIRLEDLevel(m_model));
}


void ImagePageDisplayMulti::on_pushButton_test_clicked()
{
  if (!m_timerTest) {
    m_timerTest = new QTimer(this);
    connect(m_timerTest, SIGNAL(timeout()), this, SLOT(onTimerTest()));
    m_timerTest->setSingleShot(true);
    m_timerTest->setInterval(1000);
  }
  if (ui->pushButton_test->text().contains("关闭")) {
    m_timerTest->start();
    ui->pushButton_test->setText("循环获取测试（已开启）");
  } else {
    m_timerTest->stop();
    ui->pushButton_test->setText("循环获取测试（已关闭）");
  }
}

void ImagePageDisplayMulti::onTimerTest()
{
  ////showWait();
  sendMessage(REQUEST_FLAG_GET_IPCIMAGE_DISPLAY, &m_channel, sizeof(int));
  //m_eventLoop.exec();
  ////closeWait();

  if (ui->pushButton_test->text().contains("开启")) {
    m_timerTest->start();
  }
}

void ImagePageDisplayMulti::onDayNightTimeChange(const QTime &time)
{
  Q_UNUSED(time)
  QTime start = ui->timeEdit_StartTimeOfNight->time();
  QTime stop = ui->timeEdit_EndTimeOfNight->time();
  if (start == stop) {
    stop = start.addSecs(60);
  }
  ui->timeEdit_EndTimeOfNight->setTime(stop);
}

void ImagePageDisplayMulti::onUpdatePlayVideo()
{
  updatePlayVideo(m_channel);
}

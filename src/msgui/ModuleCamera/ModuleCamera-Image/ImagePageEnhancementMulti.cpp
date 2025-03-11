#include "ImagePageEnhancementMulti.h"
#include "ui_ImagePageEnhancementMulti.h"
#include "EventLoop.h"
#include "MessageBox.h"
#include "MsCameraVersion.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MsSdkVersion.h"
#include "MsWaitting.h"
#include "MyDebug.h"
#include "centralmessage.h"
#include "channelcopydialog.h"

extern "C" {
#include "msg.h"

}

ImagePageEnhancementMulti::ImagePageEnhancementMulti(QWidget *parent)
    : AbstractImagePage(parent)
      , ui(new Ui::ImagePageEnhancementMulti)
{
  ui->setupUi(this);

  m_drawEnhanceEx = new DrawEnhancement(this);
  m_drawEnhanceEx->setColor(Qt::transparent);
  m_drawEnhanceEx->setMaxMaskCount(1);

  //IR Balance Mode
  ui->comboBox_IrBalanceMode->clear();
  ui->comboBox_IrBalanceMode->addItem(GET_TEXT("COMMON/1013", "Off"), 0);
  ui->comboBox_IrBalanceMode->addItem(GET_TEXT("COMMON/1012", "On"), 1);


  //White Balance
  ui->comboBox_WhiteBalance->clear();
  ui->comboBox_WhiteBalance->addItem(GET_TEXT("IMAGE/37344", "Auto White Balance"), 0);
  ui->comboBox_WhiteBalance->addItem(GET_TEXT("IMAGE/37345", "Manual White Balance"), 1);
  ui->comboBox_WhiteBalance->addItem(GET_TEXT("IMAGE/37346", "Incandescent Lamp"), 2);
  ui->comboBox_WhiteBalance->addItem(GET_TEXT("IMAGE/37347", "Warm Light Lamp"), 3);
  ui->comboBox_WhiteBalance->addItem(GET_TEXT("IMAGE/37348", "Natural Light"), 4);
  ui->comboBox_WhiteBalance->addItem(GET_TEXT("IMAGE/37349", "Fluorescent Lamp"), 5);
  ui->label_RedGainLevel->hide();
  ui->slider_RedGainLevel->hide();
  ui->label_BlueGainLevel->hide();
  ui->slider_BlueGainLevel->hide();

  //Digital Anti-fog Mode
  ui->comboBox_DigitalAntifogMode->clear();
  ui->comboBox_DigitalAntifogMode->addItem(GET_TEXT("COMMON/1013", "Off"), 0);
  ui->comboBox_DigitalAntifogMode->addItem(GET_TEXT("COMMON/1012", "On"), 1);
  ui->label_AntifogIntensity->hide();
  ui->slider_AntifogIntensity->hide();

  //Exposure Mode
  ui->comboBox_ExposureMode->clear();
  ui->comboBox_ExposureMode->addItem(GET_TEXT("IMAGE/37104", "Auto Mode"), 0);
  ui->comboBox_ExposureMode->addItem(GET_TEXT("IMAGE/37203", "Manual Mode"), 1);
  ui->label_ExposureTime->hide();
  ui->comboBox_ExposureTime->hide();
  ui->label_GainLevel->hide();
  ui->slider_GainLevel->hide();
  ui->slider_GainLevel->setRange(1, 100);

  //BLC/WDR/HLC
  ui->comboBox_BlcWdrHlc->clear();
  ui->comboBox_BlcWdrHlc->addItem("BLC", 0);
  ui->comboBox_BlcWdrHlc->addItem("WDR", 1);
  ui->comboBox_BlcWdrHlc->addItem("HLC", 2);

  //BLC Region
  ui->comboBox_BlcRegion->clear();
  ui->comboBox_BlcRegion->addItem(GET_TEXT("COMMON/1013", "Off"), 0);
  ui->comboBox_BlcRegion->addItem(GET_TEXT("COMMON/1015", "Customize"), 1);
  ui->comboBox_BlcRegion->addItem(GET_TEXT("IMAGE/37009", "Centre"), 2);

  //Region Type
  ui->comboBox_RegionType->clear();
  ui->comboBox_RegionType->addItem(GET_TEXT("IMAGE/37221", "Inclusive"), 0);
  ui->comboBox_RegionType->addItem(GET_TEXT("IMAGE/37222", "Exclusive"), 1);

  //Wide Dynamic Level
  ui->comboBox_WideDynamicLevel->clear();
  ui->comboBox_WideDynamicLevel->addItem(GET_TEXT("IMAGE/37236", "Low"), 0);
  ui->comboBox_WideDynamicLevel->addItem(GET_TEXT("IMAGE/37237", "High"), 1);
  ui->comboBox_WideDynamicLevel->addItem(GET_TEXT("IMAGE/37238", "Auto"), 2);

  //High Light Compensation
  ui->comboBox_HighLightCompensation->clear();
  ui->comboBox_HighLightCompensation->addItem(GET_TEXT("COMMON/1013", "Off"), 0);
  ui->comboBox_HighLightCompensation->addItem(GET_TEXT("IMAGE/37239", "General Mode"), 1);
  ui->comboBox_HighLightCompensation->addItem(GET_TEXT("IMAGE/37240", "Enhanced Mode"), 2);


  //Wide Dynamic Range
  ui->comboBox_WideDynamicRange->clear();
  ui->comboBox_WideDynamicRange->addItem(GET_TEXT("COMMON/1013", "Off"), 0);
  ui->comboBox_WideDynamicRange->addItem(GET_TEXT("COMMON/1012", "On"), 1);
  ui->comboBox_WideDynamicRange->addItem(GET_TEXT("COMMON/1015", "Customize"), 2);

  //Reduce Motion Blur
  ui->comboBox_reduceBlur->clear();
  ui->comboBox_reduceBlur->addItem(GET_TEXT("COMMON/1013", "Off"), 0);
  ui->comboBox_reduceBlur->addItem(GET_TEXT("COMMON/1012", "On"), 1);

  connect(ui->slider_AntifogIntensity, SIGNAL(valueChanged(int)), this, SLOT(onChangeDefog()));
  connect(ui->slider_RedGainLevel, SIGNAL(valueChanged(int)), this, SLOT(changeRedGain()));
  connect(ui->slider_BlueGainLevel, SIGNAL(valueChanged(int)), this, SLOT(changeBlueGain()));
  connect(ui->slider_HlcLevel, SIGNAL(valueChanged(int)), this, SLOT(changeHlcLevel()));

  onLanguageChanged();
  on_comboBox_BlcWdrHlc_activated(0);
}

ImagePageEnhancementMulti::~ImagePageEnhancementMulti()
{
  delete ui;
}

void ImagePageEnhancementMulti::initializeData(int channel)
{
  msprintf("gsjt 1");
  m_drawEnhanceEx->clearAll();
  msprintf("gsjt 2");
  setDrawWidget(m_drawEnhanceEx);

  m_channel = channel;

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

  QString cameraVersion(qMsNvr->cameraInfo(m_channel).fwversion);
  MsCameraModel cameraModel = qMsNvr->cameraRealModel(m_channel);
  int chipVersion = MsCameraVersion(cameraVersion).chipVersion();
  qDebug() << "Channel:" << m_channel << "CameraVersion:" << cameraVersion << "chipVersion:" << chipVersion;
  //安霸平台不支持《Defog Mode》和《Digital Image Stabilisation》，NVR界面安霸机型需要隐藏该选项
  if (qMsNvr->isAmba(m_channel)) {
    ui->label_DigitalAntifogMode->hide();
    ui->comboBox_DigitalAntifogMode->hide();
  }

  //MsWaitting::showGlobalWait();
  //
  //multi鱼眼，BLC配置：只有O视图支持，其他视图通道下该项置灰不可配置
  if (qMsNvr->isFisheye(m_channel)) {
    sendMessage(REQUEST_FLAG_GET_FISHEYE_MODE, (void *)&m_channel, sizeof(int));
    // int result = m_eventLoop.exec();
    // if (result < 0) {
    //   ui->comboBox_BlcRegion->setEnabled(false);
    // } else {
    //   ui->comboBox_BlcRegion->setEnabled(true);
    // }
  } else {
    ui->comboBox_BlcRegion->setEnabled(true);
  }
  //
  sendMessage(REQUEST_FLAG_GET_IPCPARAM, &m_channel, sizeof(int));
  sendMessage(REQUEST_FLAG_GET_IPCIMAGE_DISPLAY, &m_channel, sizeof(int));
}

void ImagePageEnhancementMulti::processMessage(MessageReceive *message)
{
  switch (message->type()) {
  case RESPONSE_FLAG_GET_FISHEYE_MODE:
    ON_RESPONSE_FLAG_GET_FISHEYE_MODE(message);
    break;
  case RESPONSE_FLAG_GET_IPCPARAM:
    ON_RESPONSE_FLAG_GET_IPCPARAM(message);
    break;
  case RESPONSE_FLAG_GET_IPCIMAGE_ENHANCEMENT:
    ON_RESPONSE_FLAG_GET_IPCIMAGE_ENHANCEMENT(message);
    break;
  case RESPONSE_FLAG_GET_IPC_COMMON_PARAM:
    ON_RESPONSE_FLAG_GET_IPC_COMMON_PARAM(message);
    break;
  case RESPONSE_FLAG_GET_IPCIMAGE_DISPLAY:
    ON_RESPONSE_FLAG_GET_IPCIMAGE_DISPLAY(message);
    break;
  case RESPONSE_FLAG_GET_IPC_LPR_NIGHT_MODE:
    ON_RESPONSE_FLAG_GET_IPC_LPR_NIGHT_MODE(message);
    break;
  default:
    break;
  }
}

void ImagePageEnhancementMulti::clearSettings()
{
  ui->widgetMessage->hideMessage();
  ui->comboBox_WhiteBalance->setCurrentIndex(0);
  ui->comboBox_ExposureMode->setCurrentIndex(0);
  ui->comboBox_DigitalAntifogMode->setCurrentIndex(0);
  ui->comboBox_IrBalanceMode->setCurrentIndex(0);
  ui->comboBox_BlcWdrHlc->setCurrentIndex(0);
  ui->comboBox_WideDynamicRange->setCurrentIndex(0);
  ui->comboBox_reduceBlur->setCurrentIndex(0);
  on_comboBox_ExposureMode_activated(ui->comboBox_ExposureMode->currentIndex());
  ui->label_AntifogIntensity->hide();
  ui->slider_AntifogIntensity->hide();
  ui->label_RedGainLevel->hide();
  ui->slider_RedGainLevel->hide();
  ui->label_BlueGainLevel->hide();
  ui->slider_BlueGainLevel->hide();

  ui->label_deblur->hide();
  ui->slider_deblur->hide();


  ui->label_reduceBlur->show();
  ui->comboBox_reduceBlur->show();
  ui->label_DigitalAntifogMode->show();
  ui->comboBox_DigitalAntifogMode->show();
  ui->label_ExposureMode->show();
  ui->comboBox_ExposureMode->show();
}

void ImagePageEnhancementMulti::setSettingsEnable(bool enable)
{
  ui->widgetContainer->setEnabled(enable);

  ui->pushButton_copy->setEnabled(enable);
  ui->pushButton_apply->setEnabled(enable);
}

void ImagePageEnhancementMulti::ON_RESPONSE_FLAG_GET_IPC_LPR_NIGHT_MODE(MessageReceive *message)
{
  resp_lpr_night_mode *data = reinterpret_cast<resp_lpr_night_mode *>(message->data);
  if (!data) {
    //MsWaitting::closeGlobalWait();
    qMsWarning() << "data is null";
    gEventLoopExit(0);
    return;
  }
  gEventLoopExit(0);
}

void ImagePageEnhancementMulti::ON_RESPONSE_FLAG_GET_IPCIMAGE_ENHANCEMENT(MessageReceive *message)
{
  struct resp_image_enhancement *data = (struct resp_image_enhancement *)message->data;
  if (!data) {
    qMsWarning() << "data is null.";
    //MsWaitting::closeGlobalWait();
    return;
  }
  memcpy(&m_info, &(data->imgMulti), sizeof(IMAGE_ENHANCEMENT_MULTI_S));
  initEnhancement();
  //MsWaitting::closeGlobalWait();
}


void ImagePageEnhancementMulti::ON_RESPONSE_FLAG_GET_IPCIMAGE_DISPLAY(MessageReceive *message)
{
  resp_image_display *data = (resp_image_display *)message->data;
  if (!data) {
    //MsWaitting::closeGlobalWait();
    qMsWarning() << "data is null.";
    return;
  }

  m_powerLineFrequency = static_cast<PowerLineFrequency>(data->imgMulti.scenes[0].powerlineFreq);


  //
  sendMessage(REQUEST_FLAG_GET_IPCIMAGE_ENHANCEMENT, &m_channel, sizeof(int));
}

void ImagePageEnhancementMulti::ON_RESPONSE_FLAG_GET_IPC_COMMON_PARAM(MessageReceive *message)
{
  struct resp_http_common_param *param = (struct resp_http_common_param *)message->data;
  if (!param) {
    qMsWarning() << "data is null.";
    return;
  }
  if (QString(param->info.key) == "redgain") {
    int value = QString(param->info.value).toInt();
    ui->slider_RedGainLevel->setValue(value);
  } else if (QString(param->info.key) == "bluegain") {
    int value = QString(param->info.value).toInt();
    ui->slider_BlueGainLevel->setValue(value);
  } else if (QString(param->info.key) == "defogmode") {
    int value = QString(param->info.value).toInt();
    ui->comboBox_DigitalAntifogMode->setCurrentIndex(value);
    on_comboBox_DigitalAntifogMode_activated(ui->comboBox_DigitalAntifogMode->currentIndex() && ui->comboBox_DigitalAntifogMode->isVisible());
  } else if (QString(param->info.key) == "defoglevel") {
    int value = QString(param->info.value).toInt();
    ui->slider_AntifogIntensity->setValue(value);
  }  else if (QString(param->info.key) == "hlclevel") {
    int value = QString(param->info.value).toInt();
    ui->slider_HlcLevel->setValue(value);
  } else if (QString(param->info.key) == "exposetime") {
    int value = QString(param->info.value).toInt();
    ui->comboBox_ExposureTime->setCurrentIndexFromData(value);
  } else if (QString(param->info.key) == "exposegain") {
    int value = QString(param->info.value).toInt();
    ui->slider_GainLevel->setValue(value);
  }
}

void ImagePageEnhancementMulti::showBlcWdrHlcMode()
{
  if (m_info.scenes[0].blcRegion != 0) {
    ui->comboBox_BlcWdrHlc->setCurrentIndex(0);
  } else if (m_info.scenes[0].wdrMode != 0) {
    ui->comboBox_BlcWdrHlc->setCurrentIndex(1);
  } else if (m_info.scenes[0].hlcMode) {
    ui->comboBox_BlcWdrHlc->setCurrentIndex(2);
  }
}

void ImagePageEnhancementMulti::initExposureTime()
{
  if (!m_info.scenes[0].exposureMode) {
    ui->comboBox_ExposureTime->clear();
    ui->comboBox_ExposureTime->addItem("1", 13);
    ui->comboBox_ExposureTime->addItem("1/5", 0);
    ui->comboBox_ExposureTime->addItem("1/15", 1);
    ui->comboBox_ExposureTime->addItem("1/30", 2);
    ui->comboBox_ExposureTime->addItem("1/60", 3);
    ui->comboBox_ExposureTime->addItem("1/120", 4);
    ui->comboBox_ExposureTime->addItem("1/250", 5);
    ui->comboBox_ExposureTime->addItem("1/500", 6);
    ui->comboBox_ExposureTime->addItem("1/750", 7);
    ui->comboBox_ExposureTime->addItem("1/1000", 8);
    ui->comboBox_ExposureTime->addItem("1/2000", 9);
    ui->comboBox_ExposureTime->addItem("1/4000", 10);
    ui->comboBox_ExposureTime->addItem("1/10000", 11);
    ui->comboBox_ExposureTime->addItem("1/100000", 12);
    ui->comboBox_ExposureTime->setCurrentIndexFromData(2);
  } else {
    ui->comboBox_ExposureTime->clear();
    ui->comboBox_ExposureTime->addItem("1", 13);
    ui->comboBox_ExposureTime->addItem("1/5", 0);
    ui->comboBox_ExposureTime->addItem("1/10", 1);
    ui->comboBox_ExposureTime->addItem("1/25", 2);
    ui->comboBox_ExposureTime->addItem("1/50", 3);
    ui->comboBox_ExposureTime->addItem("1/100", 4);
    ui->comboBox_ExposureTime->addItem("1/250", 5);
    ui->comboBox_ExposureTime->addItem("1/500", 6);
    ui->comboBox_ExposureTime->addItem("1/750", 7);
    ui->comboBox_ExposureTime->addItem("1/1000", 8);
    ui->comboBox_ExposureTime->addItem("1/2000", 9);
    ui->comboBox_ExposureTime->addItem("1/4000", 10);
    ui->comboBox_ExposureTime->addItem("1/10000", 11);
    ui->comboBox_ExposureTime->addItem("1/100000", 12);
    ui->comboBox_ExposureTime->setCurrentIndexFromData(2);
  }
}

void ImagePageEnhancementMulti::initEnhancement()
{
  ui->comboBox_IrBalanceMode->setCurrentIndex(m_info.scenes[0].irBalanceMode);

  ui->comboBox_WhiteBalance->setCurrentIndex(m_info.scenes[0].whitebalanceMode);
  ui->comboBox_ExposureMode->setCurrentIndex(m_info.scenes[0].exposureMode);

  ui->comboBox_BlcRegion->setCurrentIndex(m_info.scenes[0].blcRegion);

  initHighLightCompensation(m_info.scenes[0].hlcMode);
  ui->comboBox_RegionType->setCurrentIndex(m_info.scenes[0].blcRegionRange);
  ui->comboBox_WideDynamicRange->setCurrentIndexFromData(m_info.scenes[0].wdrMode);
  wdrIsOn = (m_info.scenes[0].wdrMode != 0) ? true : false;
  ui->comboBox_WideDynamicLevel->setCurrentIndex(m_info.scenes[0].wdrLevel);

  ui->timeEdit_startTime->setTime(QTime(m_info.scenes[0].wdrStartHour, m_info.scenes[0].wdrStartMin));
  ui->timeEdit_endTime->setTime(QTime(m_info.scenes[0].wdrEndHour, m_info.scenes[0].wdrEndMin));
  ui->comboBox_reduceBlur->setCurrentIndexFromData(m_info.scenes[0].reduceMotionBlur);
  ui->slider_deblur->setValue(m_info.scenes[0].deblur);

  ui->comboBox_BlcWdrHlc->setCurrentIndex(0);

  showBlcWdrHlcMode();
  on_comboBox_BlcWdrHlc_activated(ui->comboBox_BlcWdrHlc->currentIndex());
  on_comboBox_WhiteBalance_activated(ui->comboBox_WhiteBalance->currentIndex());
  on_comboBox_ExposureMode_activated(ui->comboBox_ExposureMode->currentIndex());

  ui->label_reduceBlur->show();
  ui->label_deblur->show();
  ui->comboBox_reduceBlur->show();
  ui->slider_deblur->show();
  on_comboBox_reduceBlur_activated();

  initExposureTime();
  ui->comboBox_ExposureTime->setCurrentIndexFromData(m_info.scenes[0].exposureTime);
  ui->slider_GainLevel->setValue(m_info.scenes[0].exposureGain);
  blockSignals(true);
  drawEnhanceArea();
  blockSignals(false);
  ui->slider_RedGainLevel->setValue(m_info.scenes[0].whitebalanceRedGain);
  ui->slider_BlueGainLevel->setValue(m_info.scenes[0].whitebalanceBlueGain);
  ui->comboBox_DigitalAntifogMode->setCurrentIndex(m_info.scenes[0].defogMode);
  on_comboBox_DigitalAntifogMode_activated(ui->comboBox_DigitalAntifogMode->currentIndex() && ui->comboBox_DigitalAntifogMode->isVisible());
  ui->slider_AntifogIntensity->setValue(m_info.scenes[0].antiFogIntensity);
  ui->slider_HlcLevel->setValue(m_info.scenes[0].hlcLevel);
}

void ImagePageEnhancementMulti::drawEnhanceArea()
{
  int left = 0, top = 0, right = 0, bottom = 0;
  int regiontype = ui->comboBox_BlcRegion->currentIndex();
  m_drawEnhanceEx->clearAll();
  setDrawWidget(m_drawEnhanceEx);
  if (!regiontype) {
    setDrawWidget(nullptr);
  } else if (regiontype == 1) {
    //customize
    QString area = QString("%1").arg(m_info.scenes[0].blcRegionArea);
    QStringList areaList = area.split(":");
    if (areaList.size() == 4) {
      left = areaList.at(0).toInt();
      top = areaList.at(1).toInt();
      right = areaList.at(2).toInt();
      bottom = areaList.at(3).toInt();
    }
    qDebug() << "yun debug l:" << left << " t:" << top << " r:" << right << " b:" << bottom;
    m_drawEnhanceEx->setBlcType(BlcCustomize);
    m_drawEnhanceEx->addEnhanceArea(left, top, right, bottom);
  } else {
    //centre
    m_drawEnhanceEx->setBlcType(BlcCentre);
    m_drawEnhanceEx->addCentreArea();
  }
}

void ImagePageEnhancementMulti::on_comboBox_WhiteBalance_activated(int index)
{
  const int mode = ui->comboBox_WhiteBalance->itemData(index).toInt();
  switch (mode) {
  case 0: //Auto White Balance
  case 2: //Incandescent Lamp
  case 3: //Warm Light Lamp
  case 4: //Natural Light
  case 5: //Fluorescent Lamp
    ui->label_RedGainLevel->hide();
    ui->slider_RedGainLevel->hide();
    ui->label_BlueGainLevel->hide();
    ui->slider_BlueGainLevel->hide();
    break;
  case 1: //Manual White Balance
    ui->label_RedGainLevel->show();
    ui->slider_RedGainLevel->show();
    ui->label_BlueGainLevel->show();
    ui->slider_BlueGainLevel->show();
    break;
    break;
  default:
    break;
  }
}

void ImagePageEnhancementMulti::on_comboBox_DigitalAntifogMode_activated(int index)
{
  const int mode = ui->comboBox_DigitalAntifogMode->itemData(index).toInt();
  switch (mode) {
  case 0: //Off
    ui->label_AntifogIntensity->hide();
    ui->slider_AntifogIntensity->hide();
    break;
  case 1: //On
    ui->label_AntifogIntensity->show();
    ui->slider_AntifogIntensity->show();
    break;
  }
}

void ImagePageEnhancementMulti::on_comboBox_ExposureMode_activated(int index)
{
  const int mode = ui->comboBox_ExposureMode->itemData(index).toInt();
  //避免显示超出范围，先把控件隐藏
  ui->label_ExposureTime->hide();
  ui->comboBox_ExposureTime->hide();
  ui->label_GainLevel->hide();
  ui->slider_GainLevel->hide();
  //
  switch (mode) {
  case 0: //Auto Mode
    break;
  case 1: //Manual Mode
    ui->label_ExposureTime->show();
    ui->comboBox_ExposureTime->show();
    ui->label_GainLevel->show();
    ui->slider_GainLevel->show();
    break;
  }
}

void ImagePageEnhancementMulti::on_comboBox_reduceBlur_activated()
{
  if (ui->comboBox_reduceBlur->currentData().toInt() == 0) {
    ui->label_deblur->hide();
    ui->slider_deblur->hide();
  } else {
    ui->label_deblur->show();
    ui->slider_deblur->show();
  }
}

void ImagePageEnhancementMulti::initHighLightCompensation(int type)
{
  int value = m_info.scenes[0].hlcMode;
  if (type == 0) {
    ui->comboBox_HighLightCompensation->clear();
    ui->comboBox_HighLightCompensation->addItem(GET_TEXT("COMMON/1013", "Off"), 0);
    ui->comboBox_HighLightCompensation->addItem(GET_TEXT("IMAGE/37239", "General Mode"), 1);
    ui->comboBox_HighLightCompensation->addItem(GET_TEXT("IMAGE/37240", "Enhanced Mode"), 2);
  } else {
    ui->comboBox_HighLightCompensation->clear();
    ui->comboBox_HighLightCompensation->addItem(GET_TEXT("IMAGE/37239", "General Mode"), 1);
    ui->comboBox_HighLightCompensation->addItem(GET_TEXT("IMAGE/37240", "Enhanced Mode"), 2);
    value = m_info.scenes[0].hlcMode == 0 ? 1 : m_info.scenes[0].hlcMode;
  }

  ui->comboBox_HighLightCompensation->setCurrentIndexFromData(value);
}

void ImagePageEnhancementMulti::on_comboBox_BlcRegion_activated(int index)
{
  if (!ui->comboBox_BlcRegion->isVisible())
    return;

  const int mode = ui->comboBox_BlcRegion->itemData(index).toInt();
  switch (mode) {
  case 0: //off
    ui->label_RegionType->hide();
    ui->comboBox_RegionType->hide();
    break;
  case 1: //customize
    ui->label_RegionType->show();
    ui->comboBox_RegionType->show();
    break;
  case 2: //centre
    ui->label_RegionType->hide();
    ui->comboBox_RegionType->hide();
    break;
  }
  drawEnhanceArea();
}

void ImagePageEnhancementMulti::on_comboBox_BlcWdrHlc_activated(int index)
{
  const int mode = ui->comboBox_BlcWdrHlc->itemData(index).toInt();
  hideBWHAll();
  ui->label_BlcWdrHlc->show();
  ui->comboBox_BlcWdrHlc->show();
  switch (mode) {
  case 0: //blc
    ui->label_BlcRegion->show();
    ui->comboBox_BlcRegion->show();
    break;
  case 1: //wdr
    ui->label_WideDynamicRange->show();
    ui->comboBox_WideDynamicRange->show();
    //MSHN-5657
    //1、电源频率为60Hz时，当帧率达到30fps以上（不含30），WDR & HLC的普通模式需禁用
    //2、电源频率为50Hz时，当帧率达到25fps以上（不含25），WDR & HLC的普通模式需禁用
    if ((m_powerLineFrequency == Frequency_60Hz && m_mainFrameRate > 30) || (m_powerLineFrequency == Frequency_50Hz && m_mainFrameRate > 25)) {
      ui->comboBox_WideDynamicRange->setCurrentIndexFromData(0);
      ui->comboBox_WideDynamicRange->setEnabled(false);
    } else {
      ui->comboBox_WideDynamicRange->setEnabled(true);
      initWideDynamicRange(ui->comboBox_WideDynamicRange->currentData().toInt());
    }
    break;
  case 2: //hlc
    ui->label_HighLightCompensation->show();
    ui->comboBox_HighLightCompensation->show();
    //MSHN-5657
    //1、电源频率为60Hz时，当帧率达到30fps以上（不含30），WDR & HLC的普通模式需禁用
    //2、电源频率为50Hz时，当帧率达到25fps以上（不含25），WDR & HLC的普通模式需禁用
    if ((m_powerLineFrequency == Frequency_60Hz && m_mainFrameRate > 30) || (m_powerLineFrequency == Frequency_50Hz && m_mainFrameRate > 25)) {
      int value = ui->comboBox_HighLightCompensation->currentIntData();
      if (value == 1) {
        value = 0;
      }
      ui->comboBox_HighLightCompensation->clear();
      ui->comboBox_HighLightCompensation->addItem(GET_TEXT("COMMON/1013", "Off"), 0);
      ui->comboBox_HighLightCompensation->addItem(GET_TEXT("IMAGE/37240", "Enhanced Mode"), 2);
      ui->comboBox_HighLightCompensation->setCurrentIndexFromData(value);
    }
    if (ui->comboBox_HighLightCompensation->currentData().toInt() != 0) {
      ui->label_HlcLevel->show();
      ui->slider_HlcLevel->show();
    }
    break;
  }
  qDebug()<<"gsjt comboBox_BlcRegion"<<ui->comboBox_BlcRegion->currentIndex();
  on_comboBox_BlcRegion_activated(ui->comboBox_BlcRegion->currentIndex());
}

void ImagePageEnhancementMulti::on_comboBox_HighLightCompensation_activated()
{
  if (ui->comboBox_HighLightCompensation->currentData().toInt() == 0) {
    ui->label_HlcLevel->hide();
    ui->slider_HlcLevel->hide();
  } else {
    ui->label_HlcLevel->show();
    ui->slider_HlcLevel->show();
  }
}

void ImagePageEnhancementMulti::on_comboBox_WideDynamicRange_activated(int index)
{
  const int mode = ui->comboBox_WideDynamicRange->itemData(index).toInt();
  ui->label_startTime->hide();
  ui->label_endTime->hide();
  ui->timeEdit_startTime->hide();
  ui->timeEdit_endTime->hide();
  ui->label_WideDynamicLevel->hide();
  ui->comboBox_WideDynamicLevel->hide();
  initWideDynamicRange(mode);
}

void ImagePageEnhancementMulti::initWideDynamicRange(int index)
{
  switch (index) {
  case 0: //off
    break;
  case 1: //on
    ui->label_WideDynamicLevel->show();
    ui->comboBox_WideDynamicLevel->show();
    break;
  case 2: //customize
    ui->label_startTime->show();
    ui->label_endTime->show();
    ui->timeEdit_startTime->show();
    ui->timeEdit_endTime->show();
    ui->label_WideDynamicLevel->show();
    ui->comboBox_WideDynamicLevel->show();
    break;
  }
}

void ImagePageEnhancementMulti::hideBWHAll()
{
  ui->label_BlcWdrHlc->hide();
  ui->comboBox_BlcWdrHlc->hide();
  ui->label_BlcRegion->hide();
  ui->comboBox_BlcRegion->hide();
  ui->label_RegionType->hide();
  ui->comboBox_RegionType->hide();
  ui->label_WideDynamicRange->hide();
  ui->comboBox_WideDynamicRange->hide();
  ui->label_WideDynamicLevel->hide();
  ui->comboBox_WideDynamicLevel->hide();
  ui->label_HighLightCompensation->hide();
  ui->comboBox_HighLightCompensation->hide();
  ui->label_HlcLevel->hide();
  ui->slider_HlcLevel->hide();
  ui->label_startTime->hide();
  ui->label_endTime->hide();
  ui->timeEdit_startTime->hide();
  ui->timeEdit_endTime->hide();
}

void ImagePageEnhancementMulti::onLanguageChanged()
{
  ui->label_IrBalanceMode->setText(GET_TEXT("IMAGE/37200", "IR Balance Mode"));
  ui->label_WhiteBalance->setText(GET_TEXT("IMAGE/37233", "White Balance"));
  ui->label_RedGainLevel->setText(GET_TEXT("IMAGE/37204", "Red Gain Level"));
  ui->label_BlueGainLevel->setText(GET_TEXT("IMAGE/37205", "Blue Gain Level"));

  ui->label_DigitalAntifogMode->setText(GET_TEXT("IMAGE/37207", "Defog Mode"));
  ui->label_AntifogIntensity->setText(GET_TEXT("IMAGE/37234", "Anti-fog Intensity"));

  ui->label_ExposureMode->setText(GET_TEXT("IMAGE/37209", "Exposure Mode"));
  ui->label_ExposureTime->setText(GET_TEXT("IMAGE/37210", "Exposure Time"));
  ui->label_GainLevel->setText(GET_TEXT("IMAGE/37211", "Gain Level"));

  ui->label_BlcWdrHlc->setText(GET_TEXT("IMAGE/37218", "BLC/WDR/HLC"));
  ui->label_BlcRegion->setText(GET_TEXT("IMAGE/37219", "BLC Region"));
  ui->label_RegionType->setText(GET_TEXT("IMAGE/37220", "Region Type"));
  ui->label_WideDynamicRange->setText(GET_TEXT("IMAGE/37235", "Wide Dynamic Range"));
  ui->label_WideDynamicLevel->setText(GET_TEXT("IMAGE/37223", "Wide Dynamic Level"));
  ui->label_HighLightCompensation->setText(GET_TEXT("IMAGE/37225", "High Light Compensation"));
  ui->label_HlcLevel->setText(GET_TEXT("IMAGE/37226", "HLC Level"));


  ui->label_startTime->setText(GET_TEXT("IMAGE/37320", "Start Time"));
  ui->label_endTime->setText(GET_TEXT("IMAGE/37321", "End Time"));

  ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));
  ui->pushButton_apply->setText(GET_TEXT("COMMON/1003", "Apply"));
  ui->pushButton_copy->setText(GET_TEXT("COMMON/1005", "Copy"));

  ui->label_reduceBlur->setText(GET_TEXT("IMAGE/37130", "Reduce Motion Blur"));
  ui->label_deblur->setText(GET_TEXT("IMAGE/37131", "Deblur Level"));

}

void ImagePageEnhancementMulti::ON_RESPONSE_FLAG_GET_FISHEYE_MODE(MessageReceive *message)
{
  Q_UNUSED(message)
}

void ImagePageEnhancementMulti::ON_RESPONSE_FLAG_GET_IPCPARAM(MessageReceive *message)
{
  resp_get_ipc_param *ipc_param = (resp_get_ipc_param *)message->data;
  if (ipc_param) {
    m_mainFrameRate = ipc_param->main_range.framerate;
  }
}

void ImagePageEnhancementMulti::saveEnhancement(int channel)
{
  char                         exposeregion[64] = {0};
  struct req_image_enhancement req;
  memset(&req, 0, sizeof(struct req_image_enhancement));
  req.type = IPC_IMAGE_TYPE_MULTI;
  memcpy(&req.imgMulti, &m_info, sizeof(IMAGE_ENHANCEMENT_MULTI_S));
  struct image_exposure_area area;
  memset(&area, 0, sizeof(struct image_exposure_area));

  m_drawEnhanceEx->getEnhanceArea(&area);
  snprintf(exposeregion, sizeof(exposeregion), "%d:%d:%d:%d", area.left, area.top, area.right, area.bottom);

  req.chnid                               = channel;
  req.imgMulti.scenes[0].irBalanceMode    = ui->comboBox_IrBalanceMode->currentIndex();
  req.imgMulti.scenes[0].whitebalanceMode = static_cast<IPC_WHITEBALANCE_MODE_E>(ui->comboBox_WhiteBalance->currentIndex());
  req.imgMulti.scenes[0].reduceMotionBlur = ui->comboBox_reduceBlur->currentData().toInt();
  if (req.imgMulti.scenes[0].reduceMotionBlur == 1) {
    req.imgMulti.scenes[0].deblur = ui->slider_deblur->value();
  }
  if (req.imgMulti.scenes[0].whitebalanceMode == 1) // manual white balance || schedule mode
  {
    req.imgMulti.scenes[0].whitebalanceRedGain  = ui->slider_RedGainLevel->value();
    req.imgMulti.scenes[0].whitebalanceBlueGain = ui->slider_BlueGainLevel->value();
  }
  req.imgMulti.scenes[0].defogMode  = ui->comboBox_DigitalAntifogMode->currentIndex();
  req.imgMulti.scenes[0].antiFogIntensity = ui->slider_AntifogIntensity->value();
  req.imgMulti.scenes[0].exposureMode = static_cast<IPC_EXPOSURE_MODE_E>(ui->comboBox_ExposureMode->currentIndex());
  if (req.imgMulti.scenes[0].exposureMode == 1) // manual Mode
  {
    req.imgMulti.scenes[0].exposureTime = static_cast<IPC_EXPOSURE_TIME_E>(ui->comboBox_ExposureTime->currentData().toInt());
    req.imgMulti.scenes[0].exposureGain = ui->slider_GainLevel->value();
  }

  if (ui->comboBox_BlcWdrHlc->currentIndex() == 0) // blc
  {
    req.imgMulti.scenes[0].blcRegion = static_cast<IPC_BLC_REGION_E>(ui->comboBox_BlcRegion->currentIndex());
    if (req.imgMulti.scenes[0].blcRegion == 0) // off
    {
      req.imgMulti.scenes[0].wdrMode = IPC_WDR_MODE_OFF;
      req.imgMulti.scenes[0].hlcMode = IPC_HLC_MODE_OFF;
      snprintf(req.imgMulti.scenes[0].blcRegionArea, sizeof(req.imgMulti.scenes[0].blcRegionArea), "%s", "0:0:0:0");
    } else {
      req.imgMulti.scenes[0].wdrMode = IPC_WDR_MODE_OFF;
      req.imgMulti.scenes[0].hlcMode = IPC_HLC_MODE_OFF;
      if (req.imgMulti.scenes[0].blcRegion == 1) // customize
      {
        snprintf(req.imgMulti.scenes[0].blcRegionArea, sizeof(req.imgMulti.scenes[0].blcRegionArea), "%s", exposeregion);
        req.imgMulti.scenes[0].blcRegionRange = static_cast<IPC_BLC_REGION_RANGE_E>(ui->comboBox_RegionType->currentIndex());
      } else // centre
      {
        snprintf(req.imgMulti.scenes[0].blcRegionArea, sizeof(req.imgMulti.scenes[0].blcRegionArea), "%s", "0:0:0:0");
      }
    }
  } else if (ui->comboBox_BlcWdrHlc->currentIndex() == 1) // wdr
  {
    req.imgMulti.scenes[0].wdrMode = static_cast<IPC_WDR_MODE_E>(ui->comboBox_WideDynamicRange->currentData().toInt());
    if (req.imgMulti.scenes[0].wdrMode != 0) // off
    {
      req.imgMulti.scenes[0].blcRegion = IPC_BLC_REGION_OFF;
      req.imgMulti.scenes[0].hlcMode   = IPC_HLC_MODE_OFF;
      if (req.imgMulti.scenes[0].wdrMode == 1) // on
      {
        req.imgMulti.scenes[0].wdrLevel = static_cast<IPC_WDR_LEVEL_E>(ui->comboBox_WideDynamicLevel->currentIndex());
        // setCommonParam(channel, "aflevel", ui->slider_AntiflickerLevel->value());
      } else // customize
      {
        QTime timeStart                     = ui->timeEdit_startTime->time();
        QTime timeEnd                       = ui->timeEdit_endTime->time();
        req.imgMulti.scenes[0].wdrStartHour = timeStart.hour();
        req.imgMulti.scenes[0].wdrStartMin  = timeStart.minute();
        req.imgMulti.scenes[0].wdrEndHour   = timeEnd.hour();
        req.imgMulti.scenes[0].wdrEndMin    = timeEnd.minute();
        req.imgMulti.scenes[0].wdrLevel     = static_cast<IPC_WDR_LEVEL_E>(ui->comboBox_WideDynamicLevel->currentIndex());
        // setCommonParam(channel, "aflevel", ui->slider_AntiflickerLevel->value());
      }
    }
  } else // hlc
  {
    req.imgMulti.scenes[0].hlcMode = static_cast<IPC_HLC_MODE_E>(ui->comboBox_HighLightCompensation->currentData().toInt());
    if (req.imgMulti.scenes[0].hlcLevel != 0) {
      req.imgMulti.scenes[0].blcRegion = IPC_BLC_REGION_OFF;
      req.imgMulti.scenes[0].wdrMode   = IPC_WDR_MODE_OFF;
      req.imgMulti.scenes[0].hlcLevel  = ui->slider_HlcLevel->value();
    }
  }


  sendMessageOnly(REQUEST_FLAG_SET_IPCIMAGE_ENHANCEMENT, &req, sizeof(struct req_image_enhancement));
}

void ImagePageEnhancementMulti::copyEnhancement()
{
  int chnid = m_copyChannelList.takeFirst();
  saveEnhancement(chnid);
}

void ImagePageEnhancementMulti::on_pushButton_back_clicked()
{
  back();
}

void ImagePageEnhancementMulti::on_pushButton_apply_clicked()
{
  //MsWaitting::showGlobalWait();
  if (ui->comboBox_ExposureMode->currentIndex() == 1 && ui->comboBox_WideDynamicRange->currentIndex() != 0) {
    QString questionText;
    if (wdrIsOn) {
      questionText = GET_TEXT("IMAGE/37245", "WDR will be disabled if Exposure Mode is Manual Mode, continue?");
    } else {
      questionText = GET_TEXT("IMAGE/37244", "Exposure Mode will be set to auto mode if WDR is enabled, continue?");
    }
    const int result = MessageBox::question(this, questionText);
    if (result == MessageBox::Yes) {
      if (wdrIsOn) {
        ui->comboBox_WideDynamicRange->setCurrentIndexFromData(0);
        on_comboBox_WideDynamicRange_activated(0);
      } else {
        ui->comboBox_ExposureMode->setCurrentIndexFromData(0);
        on_comboBox_ExposureMode_activated(0);
      }
    } else {
      //MsWaitting::closeGlobalWait();
      return;
    }
  }

  if (m_copyChannelList.isEmpty()) {
    m_copyChannelList.append(m_channel);
  }
  do {
    copyEnhancement();
  } while (!m_copyChannelList.isEmpty());

  initializeData(m_channel);
}

void ImagePageEnhancementMulti::on_pushButton_copy_clicked()
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

void ImagePageEnhancementMulti::getStructMulti(req_image_enhancement *req)
{
  memset(req, 0, sizeof(struct req_image_enhancement));
  req->type = IPC_IMAGE_TYPE_MULTI;
  memcpy(&req->imgMulti, &m_info, sizeof(IMAGE_ENHANCEMENT_MULTI_S));
  req->chnid = m_channel;
}

void ImagePageEnhancementMulti::onChangeDefog()
{
  struct req_image_enhancement req;
  getStructMulti(&req);
  req.imgMulti.scenes[0].antiFogIntensity = ui->slider_AntifogIntensity->value();
  sendMessageOnly(REQUEST_FLAG_SET_IPCIMAGE_ENHANCEMENT, &req, sizeof(struct req_image_enhancement));
}

void ImagePageEnhancementMulti::changeRedGain()
{
  struct req_image_enhancement req;
  getStructMulti(&req);
  req.imgMulti.scenes[0].whitebalanceRedGain = ui->slider_RedGainLevel->value();
  sendMessageOnly(REQUEST_FLAG_SET_IPCIMAGE_ENHANCEMENT, &req, sizeof(struct req_image_enhancement));
}

void ImagePageEnhancementMulti::changeBlueGain()
{
  struct req_image_enhancement req;
  getStructMulti(&req);
  req.imgMulti.scenes[0].whitebalanceBlueGain = ui->slider_BlueGainLevel->value();
  sendMessageOnly(REQUEST_FLAG_SET_IPCIMAGE_ENHANCEMENT, &req, sizeof(struct req_image_enhancement));
}

void ImagePageEnhancementMulti::changeHlcLevel()
{
  if (m_info.scenes[0].hlcMode) {
    struct req_image_enhancement req;
    getStructMulti(&req);
    req.imgMulti.scenes[0].hlcLevel = ui->slider_HlcLevel->value();
    sendMessageOnly(REQUEST_FLAG_SET_IPCIMAGE_ENHANCEMENT, &req, sizeof(struct req_image_enhancement));
  }
}

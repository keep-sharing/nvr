#include "ImagePageDayNightMulti.h"
#include "ui_ImagePageDayNightMulti.h"
#include "DayNightSchedule.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MsSdkVersion.h"
#include "MsWaitting.h"
#include "centralmessage.h"
#include "channelcopydialog.h"
#include "daynightscheedit.h"
#include <QtDebug>

ImagePageDayNightMulti::ImagePageDayNightMulti(QWidget *parent)
    : AbstractImagePage(parent)
      , ui(new Ui::ImagePageDayNightMulti)
{
  ui->setupUi(this);

  //initialize table
  QStringList headerList;
  headerList << "";
  headerList << GET_TEXT("IMAGE/162001", "Template");
  headerList << GET_TEXT("IMAGE/37300", "Time");
  headerList << GET_TEXT("IMAGE/37302", "Minimum Shutter");
  headerList << GET_TEXT("IMAGE/37303", "Maximum Shutter");
  headerList << GET_TEXT("IMAGE/37304", "Limit Gain Level");
  headerList << GET_TEXT("IMAGE/37305", "IR-CUT Latency");
  headerList << GET_TEXT("IMAGE/37306", "IR-CUT");
  headerList << GET_TEXT("IMAGE/37307", "IR LED");
  headerList << GET_TEXT("IMAGE/37311", "Color Mode");
  headerList << GET_TEXT("COMMON/1019", "Edit");
  ui->tableView->setHorizontalHeaderLabels(headerList);
  ui->tableView->setColumnCount(headerList.size());
  ui->tableView->hideColumn(ColumnCheck);

  ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));
  ui->pushButton_apply->setText(GET_TEXT("COMMON/1003", "Apply"));
  ui->pushButton_copy->setText(GET_TEXT("COMMON/1005", "Copy"));

  //delegate
  ui->tableView->setItemDelegateForColumn(ColumnEdit, new ItemButtonDelegate(QPixmap(":/common/common/edit.png"), this));
  //sort
  ui->tableView->setSortingEnabled(false);
  //connect
  connect(ui->tableView, SIGNAL(itemClicked(int, int)), this, SLOT(slotEditDayNightInfo(int, int)));
}

ImagePageDayNightMulti::~ImagePageDayNightMulti()
{
  delete ui;
}

void ImagePageDayNightMulti::initializeData(int channel)
{
  m_channel = channel;

  clearSettings();
  setSettingsEnable(false);
  //MsWaitting::showGlobalWait();

  if (!isChannelConnected()) {
    ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20102", "This channel is not connected."));
    //MsWaitting::closeGlobalWait();
    return;
  }
  if (!qMsNvr->isMsCamera(m_channel)) {
    ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20107", "This channel does not support this function."));
    //MsWaitting::closeGlobalWait();
    return;
  }

  setSettingsEnable(true);

  m_isWhite = false;
  sendMessage(REQUEST_FLAG_GET_DAY_NIGHT_INFO, &m_channel, sizeof(int));
}

void ImagePageDayNightMulti::processMessage(MessageReceive *message)
{
  switch (message->type()) {
  case RESPONSE_FLAG_GET_DAY_NIGHT_INFO:
    ON_RESPONSE_FLAG_GET_DAY_NIGHT_INFO(message);
    break;
  default:
    break;
  }
}

void ImagePageDayNightMulti::clearSettings()
{
  ui->widgetMessage->hideMessage();
  ui->tableView->clearContent();
}

void ImagePageDayNightMulti::setSettingsEnable(bool enable)
{
  ui->tableView->setEnabled(enable);
  ui->pushButton_copy->setEnabled(enable);
  ui->pushButton_apply->setEnabled(enable);
}

void ImagePageDayNightMulti::ON_RESPONSE_FLAG_GET_DAY_NIGHT_INFO(MessageReceive *message)
{
  //MsWaitting::closeGlobalWait();
  set_image_day_night_str *data = static_cast<set_image_day_night_str *>(message->data);
  if (!data) {
    qWarning() << "ImagePageDayNightMulti::ON_RESPONSE_FLAG_GET_DAY_NIGHT_INFO, data is null.";
    return;
  }
  memset(&m_info, 0, sizeof(set_image_day_night_str));
  memcpy(&m_info, data, sizeof(set_image_day_night_str));
  initDayNight();
}

QString ImagePageDayNightMulti::showTime(int id)
{
  QString time;
  QString startMinute;
  QString endMinute;
  switch (id) {
  case 0:
    time.append("Night");
    break;
  case 1:
    time.append("Day");
    break;
  default:
    time = "-";
    break;
  }
  return time;
}

QString ImagePageDayNightMulti::showIRLedOffOnStatus(int type)
{
  QString show;
  switch (type) {
  case 0:
    if (m_isWhite)
      show.append(GET_TEXT("IMAGE/37366", "All LED Off"));
    else
      show.append(GET_TEXT("IMAGE/37316", "Off"));
    break;
  case 1:
    if (m_isWhite)
      show.append(GET_TEXT("IMAGE/37367", "IR LED On"));
    else
      show.append(GET_TEXT("IMAGE/37317", "On"));
    break;
  case 2:
    show.append(GET_TEXT("IMAGE/37368", "White LED On"));
    break;
  }
  return show;
}

QString ImagePageDayNightMulti::showOffOnStatus(int type)
{
  QString show;
  switch (type) {
  case 0:
    show.append(GET_TEXT("IMAGE/37316", "Off"));
    break;
  case 1:
    show.append(GET_TEXT("IMAGE/37317", "On"));
    break;
  }
  return show;
}

QString ImagePageDayNightMulti::showColorMode(int type)
{
  QString show;
  switch (type) {
  case 0:
    show.append("B/W");
    break;
  case 1:
    show.append("Color");
    break;
  }
  return show;
}

QString ImagePageDayNightMulti::showShutter(int num)
{
  QString show;
  if (!m_exposureCtrl) {
    switch (num) {
    case 0:
      show.append("1/5");
      break;
    case 1:
      show.append("1/15");
      break;
    case 2:
      show.append("1/30");
      break;
    case 3:
      show.append("1/60");
      break;
    case 4:
      show.append("1/120");
      break;
    case 5:
      show.append("1/250");
      break;
    case 6:
      show.append("1/500");
      break;
    case 7:
      show.append("1/750");
      break;
    case 8:
      show.append("1/1000");
      break;
    case 9:
      show.append("1/2000");
      break;
    case 10:
      show.append("1/4000");
      break;
    case 11:
      show.append("1/10000");
      break;
    case 12:
      show.append("1/100000");
      break;
    case 13:
      show.append("1");
      break;
    default:
      break;
    }
  } else {
    switch (num) {
    case 0:
      show.append("1/5");
      break;
    case 1:
      show.append("1/10");
      break;
    case 2:
      show.append("1/25");
      break;
    case 3:
      show.append("1/50");
      break;
    case 4:
      show.append("1/100");
      break;
    case 5:
      show.append("1/250");
      break;
    case 6:
      show.append("1/500");
      break;
    case 7:
      show.append("1/750");
      break;
    case 8:
      show.append("1/1000");
      break;
    case 9:
      show.append("1/2000");
      break;
    case 10:
      show.append("1/4000");
      break;
    case 11:
      show.append("1/10000");
      break;
    case 12:
      show.append("1/100000");
      break;
    case 13:
      show.append("1");
      break;
    default:
      break;
    }
  }
  return show;
}

void ImagePageDayNightMulti::deleteListAll()
{
  int i = 0;
  int row = ui->tableView->rowCount();
  for (i = row; i >= 0; i--)
    ui->tableView->removeRow(i);
}

void ImagePageDayNightMulti::initDayNight()
{
  int i = 0, row = 0;
  deleteListAll();
  m_exposureCtrl = true;
  for (i = 0; i < MAX_IMAGE_DAY_NIGHT_DEFUALT; i++) {

    ui->tableView->insertRow(row);
    ui->tableView->setItemIntValue(row, ColumnId, i);

    QString templateStr = i < 2 ? "-" : QString("%1").arg(i - 1);
    ui->tableView->setItemText(row, ColumnId, templateStr);

    QString time = showTime(i);
    ui->tableView->setItemText(row, ColumnTime, time);

    QString minShutter = showShutter(m_info.imgMulti.scenes[0].minShutter[i]);
    ui->tableView->setItemText(row, ColumnMinimumShutter, minShutter);

    QString maxShutter = showShutter(m_info.imgMulti.scenes[0].maxShutter[i]);
    ui->tableView->setItemText(row, ColumnMaximumShutter, maxShutter);

    ui->tableView->setItemIntValue(row, ColumnLimitGainLevel, m_info.imgMulti.scenes[0].limitGain[i]);

    QString irCutLatency = QString("%1s").arg(m_info.imgMulti.scenes[0].irCutInterval[i]);
    ui->tableView->setItemText(row, ColumnIrCutLatency, irCutLatency);

    QString irCutState = showOffOnStatus(m_info.imgMulti.scenes[0].irCutStatus[i]);
    ui->tableView->setItemText(row, ColumnIrCut, irCutState);

    QString irLedState = showIRLedOffOnStatus(m_info.imgMulti.scenes[0].irLedStatus[i]);
    ui->tableView->setItemText(row, ColumnIrLed, irLedState);

    QString colorMode = showColorMode(m_info.imgMulti.scenes[0].colorMode[i]);
    ui->tableView->setItemText(row, ColumnColorMode, colorMode);

    row++;
  }
}

void ImagePageDayNightMulti::resizeEvent(QResizeEvent *)
{
  int columnWidth = width() / (ui->tableView->columnCount() - 1);
  for (int i = ColumnId; i < ColumnEdit; ++i) {
    ui->tableView->setColumnWidth(i, columnWidth);
  }
}

void ImagePageDayNightMulti::closeWait()
{
  //MsWaitting::closeGlobalWait();
}

void ImagePageDayNightMulti::slotEditDayNightInfo(int row, int column)
{
  int id = ui->tableView->itemIntValue(row, ColumnId);
  qDebug() << "row=" << row << ", column=" << column << ", id=" << id;

  if (column == ColumnEdit) {
    DayNightScheEdit dayNightSche(m_isWhite, this);
    dayNightSche.setExposureCtrl(m_exposureCtrl);
    connect(&dayNightSche, SIGNAL(sigEditSche()), this, SLOT(updateSche()));
    dayNightSche.initDayNightEditInfoMulti(&m_info.imgMulti.scenes[0], id,  m_info.type);
    dayNightSche.exec();
  }
}

void ImagePageDayNightMulti::updateSche()
{
  //    initializeData(m_channel);
  initDayNight();
}

void ImagePageDayNightMulti::saveDayNightSche()
{
  struct set_image_day_night_str info;
  memcpy(&info, &m_info, sizeof(set_image_day_night_str));
  info.chanid = m_copyChannelList.takeFirst();
  info.imgMulti.scenes[0].valid[0] = 1;
  info.imgMulti.scenes[0].valid[1] = 1;
  sendMessage(REQUEST_FLAG_SET_DAY_NIGHT_INFO, &info, sizeof(struct set_image_day_night_str));
}

void ImagePageDayNightMulti::on_pushButton_back_clicked()
{
  back();
}

void ImagePageDayNightMulti::on_pushButton_apply_clicked()
{
  //MsWaitting::showGlobalWait();

  if (m_copyChannelList.isEmpty()) {
    m_copyChannelList.append(m_channel);
  }

  do {
    saveDayNightSche();
    qApp->processEvents();
  } while (!m_copyChannelList.isEmpty());

  QTimer::singleShot(1000, this, SLOT(closeWait()));
}

void ImagePageDayNightMulti::on_pushButton_copy_clicked()
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

#include "OptimalSplicingDistance.h"
#include "ui_OptimalSplicingDistance.h"
#include "centralmessage.h"
#include "MsCameraVersion.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "SubControl.h"
#include <QDesktopWidget>
#include <QMouseEvent>
#include <QtDebug>


OptimalSplicingDistance::OptimalSplicingDistance(QWidget *parent)
    : BaseDialog(parent)
      , ui(new Ui::OptimalSplicingDistance)
{
  ui->setupUi(this);

  ui->horizontalSliderOptimal->setTextColor(QColor("#FFFFFF"));
  ui->horizontalSliderOptimal->setShowValue(false);


  connect(ui->horizontalSliderOptimal, SIGNAL(valueChanged(int)), this, SLOT(onSetImageInfo()));
  connect(ui->horizontalSliderOptimal, SIGNAL(valueChanged(int)), this, SLOT(onSliderChange(int)));
  connect(ui->doubleSpinBoxOptimal, SIGNAL(valueChanged(double)), this, SLOT(onDoubleSpinBoxChange(double)));

  //0.5秒内连续设置多次数值，只发送最后一次
  m_sendTimer = new QTimer(this);
  m_sendTimer->setSingleShot(true);
  m_sendTimer->setInterval(500);
  connect(m_sendTimer, SIGNAL(timeout()), this, SLOT(onSendTimer()));

  //
  ui->label_title->installEventFilter(this);

  connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
  onLanguageChanged();
}

OptimalSplicingDistance::~OptimalSplicingDistance()
{
  delete ui;
}

void OptimalSplicingDistance::showImageInfo(int channel, const QRect &videoGeometry)
{
  QRect screenRect = SubControl::instance()->logicalMainScreenGeometry();
  QPoint p(videoGeometry.left() - ui->widget_Optimal->width(), videoGeometry.top());
  if (p.x() < screenRect.left()) {
    p.setX(videoGeometry.right());
  }
  if (p.y() + ui->widget_Optimal->height() > screenRect.bottom()) {
    p.setY(screenRect.bottom() - ui->widget_Optimal->height());
  }
  if (p.x() >= screenRect.right()) {
    p.setX(videoGeometry.x());
  }
  if (p.y() >= screenRect.bottom()) {
    p.setY(videoGeometry.y());
  }
  ui->widget_Optimal->move(p);
  //
  m_currentChannel = channel;

  sendMessage(REQUEST_FLAG_GET_IPC_IMAGE_SPLICEDISTANCE, (void *)&m_currentChannel, sizeof(int));
}

void OptimalSplicingDistance::processMessage(MessageReceive *message)
{
  switch (message->type()) {
  case RESPONSE_FLAG_GET_IPC_IMAGE_SPLICEDISTANCE:
    ON_RESPONSE_FLAG_GET_IPC_IMAGE_SPLICEDISTANCE(message);
    break;
  }
}

void OptimalSplicingDistance::ON_RESPONSE_FLAG_GET_IPC_IMAGE_SPLICEDISTANCE(MessageReceive *message)
{
  if (message->data == nullptr) {
    return;
  }
  IMAGE_SPILCEDISTANCE_S *info = (IMAGE_SPILCEDISTANCE_S *)message->data;
  if (info->chnId != m_currentChannel) {
    return;
  }
  m_sendChange = false;
  ui->horizontalSliderOptimal->setValue(info->distance * 2);
  m_sendChange = true;
  QString strDebug = QString("ON_RESPONSE_FLAG_GET_IPC_IMAGE_SPLICEDISTANCE, channel: %1\n").arg(m_currentChannel);
  strDebug.append(QString("horizontalSliderOptimal: %1(%2)\n").arg(ui->doubleSpinBoxOptimal->value()).arg(info->distance));
  qDebug() << strDebug;
}

void OptimalSplicingDistance::showEvent(QShowEvent *event)
{
  Q_UNUSED(event)

  showMaximized();

  BaseDialog::showEvent(event);
}

bool OptimalSplicingDistance::eventFilter(QObject *object, QEvent *event)
{
  if (object == ui->label_title) {
    switch (event->type()) {
    case QEvent::MouseButtonPress: {
      m_titlePressed = true;
      QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
      m_titlePressedDistance = mouseEvent->globalPos() - ui->widget_Optimal->pos();
      ui->label_title->setCursor(Qt::ClosedHandCursor);
      break;
    }
    case QEvent::MouseButtonRelease: {
      m_titlePressed = false;
      ui->label_title->setCursor(Qt::OpenHandCursor);
      break;
    }
    case QEvent::MouseMove: {
      if (m_titlePressed) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        ui->widget_Optimal->move(mouseEvent->globalPos() - m_titlePressedDistance);
      }
      break;
    }
    default:
      break;
    }
  }
  return BaseDialog::eventFilter(object, event);
}

void OptimalSplicingDistance::mousePressEvent(QMouseEvent *event)
{
  if (!ui->widget_Optimal->geometry().contains(event->pos())) {
    on_pushButton_close_clicked();
  }
  BaseDialog::mousePressEvent(event);
}

void OptimalSplicingDistance::escapePressed()
{
  on_pushButton_close_clicked();
}

bool OptimalSplicingDistance::isMoveToCenter()
{
  return false;
}

bool OptimalSplicingDistance::isAddToVisibleList()
{
  return true;
}
void OptimalSplicingDistance::onLanguageChanged()
{
  ui->label_title->setText(GET_TEXT("STATUS/177013", "Optimal Splicing Distance"));

  ui->pushButton_default->setText(GET_TEXT("COMMON/1050", "Default"));
  ui->pushButton_close->setText(GET_TEXT("PTZDIALOG/21005", "Close"));
}

void OptimalSplicingDistance::onSetImageInfo()
{
  if (!m_sendChange) {
    return;
  }

  m_sendTimer->start();
}

void OptimalSplicingDistance::onSendTimer()
{
  IMAGE_SPILCEDISTANCE_S info;
  memset(&info, 0, sizeof(info));
  info.chnId = m_currentChannel;
  info.distance = ui->doubleSpinBoxOptimal->value();
  sendMessageOnly(REQUEST_FLAG_SET_IPC_IMAGE_SPLICEDISTANCE, (void *)&info, sizeof(info));
}

void OptimalSplicingDistance::on_pushButton_default_clicked()
{
  ui->horizontalSliderOptimal->setValue(20);
}

void OptimalSplicingDistance::on_pushButton_close_clicked()
{
  close();
}

void OptimalSplicingDistance::onSliderChange(int value)
{
  ui->doubleSpinBoxOptimal->setValue(double(value) / 2);
}

void OptimalSplicingDistance::onDoubleSpinBoxChange(double value)
{
  ui->horizontalSliderOptimal->setValue(int(value * 2));
}

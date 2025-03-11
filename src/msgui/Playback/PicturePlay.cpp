#include "PicturePlay.h"
#include "ui_PicturePlay.h"
#include "centralmessage.h"
#include "MsLanguage.h"
#include "MyFileSystemDialog.h"
#include "PlaybackPictureList.h"
#include <QPainter>

PicturePlay *PicturePlay::s_picturePlay = nullptr;
PicturePlay::PicturePlay(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PicturePlay)
{
    ui->setupUi(this);

    PicturePlay::s_picturePlay = this;

    connect(ui->toolButton_previousPlay, SIGNAL(clicked(bool)), this, SLOT(onToolButtonPreviousPlayClicked()));
    connect(ui->toolButton_postPlay, SIGNAL(clicked(bool)), this, SLOT(onToolButtonPostPlayClicked()));
    connect(ui->toolButton_previous, SIGNAL(clicked(bool)), this, SLOT(onToolButtonPreviousClicked()));
    connect(ui->toolButton_next, SIGNAL(clicked(bool)), this, SLOT(onToolButtonNextClicked()));

    m_autoPlayTimer = new QTimer(this);
    connect(m_autoPlayTimer, SIGNAL(timeout()), this, SLOT(onAutoPlayTimer()));
    m_autoPlayTimer->setInterval(1000);

    connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
    onLanguageChanged();
}

PicturePlay::~PicturePlay()
{
    s_picturePlay = nullptr;
    delete ui;
}

PicturePlay *PicturePlay::instance()
{
    return s_picturePlay;
}

void PicturePlay::initializeData()
{
    ui->widget_bar->setEnabled(true);
}

void PicturePlay::showImageInfo(const QString &info)
{
    ui->label_size->setText(info);
}

void PicturePlay::showImage(MessageReceive *message)
{
    m_image = message->image1;

    QString info = QString("%1, %2:%3")
                       .arg(GET_TEXT("PICTUREBACKUP/102005", "Resolution:%1*%2").arg(m_image.width()).arg(m_image.height()))
                       .arg(GET_TEXT("COMMON/1053", "Size"))
                       .arg(MyFileSystemDialog::bytesString(message->header.size));
    ui->label_size->setText(info);

    update();
}

void PicturePlay::clear()
{
    m_image = QImage();
    ui->label_size->clear();
    ui->widget_bar->setEnabled(false);
    update();
}

void PicturePlay::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    if (m_image.isNull()) {
        return;
    }
    painter.drawImage(ui->widget_pixmap->rect(), m_image);
}

void PicturePlay::onLanguageChanged()
{
    if (ui->toolButton_previousPlay->property("pause").toBool()) {
        ui->toolButton_previousPlay->setToolTip(GET_TEXT("PLAYBACK/80003", "Pause"));
    } else {
        ui->toolButton_previousPlay->setToolTip(GET_TEXT("PLAYBACK/80033", "Prevplay"));
    }
    if (ui->toolButton_postPlay->property("pause").toBool()) {
        ui->toolButton_postPlay->setToolTip(GET_TEXT("PLAYBACK/80003", "Pause"));
    } else {
        ui->toolButton_postPlay->setToolTip(GET_TEXT("PLAYBACK/80034", "Nextplay"));
    }
    ui->toolButton_previous->setToolTip(GET_TEXT("PLAYBACK/80036", "Prev"));
    ui->toolButton_next->setToolTip(GET_TEXT("WIZARD/11000", "Next"));
}

void PicturePlay::onAutoPlayTimer()
{
    bool result = true;
    if (m_isReverse) {
        result = PlaybackPictureList::instance()->playPrevious();
    } else {
        result = PlaybackPictureList::instance()->playNext();
    }
    if (!result) {
        m_autoPlayTimer->stop();

        ui->toolButton_previousPlay->setProperty("pause", false);
        ui->toolButton_previousPlay->style()->polish(ui->toolButton_previousPlay);
        ui->toolButton_previousPlay->setToolTip(GET_TEXT("PLAYBACK/80033", "Prevplay"));

        ui->toolButton_postPlay->setProperty("pause", false);
        ui->toolButton_postPlay->style()->polish(ui->toolButton_postPlay);
        ui->toolButton_postPlay->setToolTip(GET_TEXT("PLAYBACK/80034", "Nextplay"));
    }
}

void PicturePlay::onToolButtonPreviousPlayClicked()
{
    m_isReverse = true;

    bool pause = ui->toolButton_previousPlay->property("pause").toBool();
    ui->toolButton_previousPlay->setProperty("pause", !pause);
    ui->toolButton_previousPlay->style()->polish(ui->toolButton_previousPlay);
    if (pause) {
        ui->toolButton_previousPlay->setToolTip(GET_TEXT("PLAYBACK/80033", "Prevplay"));
        m_autoPlayTimer->stop();
    } else {
        ui->toolButton_previousPlay->setToolTip(GET_TEXT("PLAYBACK/80003", "Pause"));

        ui->toolButton_postPlay->setProperty("pause", false);
        ui->toolButton_postPlay->style()->polish(ui->toolButton_postPlay);
        ui->toolButton_postPlay->setToolTip(GET_TEXT("PLAYBACK/80034", "Nextplay"));
        m_autoPlayTimer->start();
    }
}

void PicturePlay::onToolButtonPostPlayClicked()
{
    m_isReverse = false;

    bool pause = ui->toolButton_postPlay->property("pause").toBool();
    ui->toolButton_postPlay->setProperty("pause", !pause);
    ui->toolButton_postPlay->style()->polish(ui->toolButton_postPlay);
    if (pause) {
        ui->toolButton_postPlay->setToolTip(GET_TEXT("PLAYBACK/80034", "Nextplay"));
        m_autoPlayTimer->stop();
    } else {
        ui->toolButton_postPlay->setToolTip(GET_TEXT("PLAYBACK/80003", "Pause"));

        ui->toolButton_previousPlay->setProperty("pause", false);
        ui->toolButton_previousPlay->style()->polish(ui->toolButton_previousPlay);
        ui->toolButton_previousPlay->setToolTip(GET_TEXT("PLAYBACK/80033", "Prevplay"));
        m_autoPlayTimer->start();
    }
}

void PicturePlay::onToolButtonPreviousClicked()
{
    PlaybackPictureList::instance()->playPrevious();
}

void PicturePlay::onToolButtonNextClicked()
{
    PlaybackPictureList::instance()->playNext();
}

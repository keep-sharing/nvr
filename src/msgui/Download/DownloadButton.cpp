#include "DownloadButton.h"
#include "ui_DownloadButton.h"
#include "DownloadPanel.h"
#include "MsLanguage.h"
#include "normallabel.h"
#include <QMouseEvent>
#include <QMovie>

DownloadButton::DownloadButton(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DownloadButton)
{
    ui->setupUi(this);

    m_movie = new QMovie(this);
    m_movie->setFileName(":/download/download/download.gif");
    m_movie->start();
    m_movie->stop();

    ui->labelGif->setMovie(m_movie);

    m_labelError = new NormalLabel(this);
    m_labelError->resize(13, 13);
    m_labelError->setScaledContents(true);
    m_labelError->setPixmap(QPixmap(":/download/download/event.png"));
    m_labelError->hide();

    //
    m_list = new DownloadPanel(this);

    //
    connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
    onLanguageChanged();
}

DownloadButton::~DownloadButton()
{
    delete ui;
}

void DownloadButton::startMovie()
{
    m_movie->start();
}

void DownloadButton::stopMovie()
{
    m_movie->stop();
}

void DownloadButton::setErrorVisible(bool visible)
{
    if (visible) {
        QPoint p = ui->labelGif->pos();
        m_labelError->move(p.x(), p.y() + ui->labelGif->height() / 3 + 5);
    }
    m_labelError->setVisible(visible);
}

void DownloadButton::mousePressEvent(QMouseEvent *event)
{
    if (ui->labelGif->geometry().contains(event->pos())) {
        showList();
    }
}

void DownloadButton::showList()
{
    QPoint p = ui->labelGif->mapToGlobal(QPoint(0, 0));
    p.setX(p.x() + ui->labelGif->width() / 2 - m_list->width() / 3 - 10);
    p.setY(p.y() + ui->labelGif->height() / 2);
    m_list->move(p);
    m_list->show();
}

void DownloadButton::onLanguageChanged()
{
    ui->labelGif->setToolTip(GET_TEXT("DOWNLOAD/60101", "Download Progress"));

    m_list->onLanguageChanged();
}

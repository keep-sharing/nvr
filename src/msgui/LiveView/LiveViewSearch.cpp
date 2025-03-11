#include "LiveViewSearch.h"
#include "ui_LiveViewSearch.h"
#include "MyDebug.h"
#include "centralmessage.h"
#include "MsLanguage.h"
#include <QDesktopWidget>

LiveViewSearch::LiveViewSearch(QWidget *parent)
    : BaseShadowDialog(parent)
    , ui(new Ui::LiveViewSearch)
{
    ui->setupUi(this);

    ui->deviceSearch->setShowInLiveView();
    connect(ui->deviceSearch, SIGNAL(sig_back()), this, SLOT(close()));

    const QRect &screenRect = qApp->desktop()->geometry();
    const QRect &rc = QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, QSize(screenRect.width() / 5 * 4, screenRect.height() / 4 * 3), screenRect);
    setGeometry(rc);

    //
    connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
    onLanguageChanged();
}

LiveViewSearch::~LiveViewSearch()
{
    delete ui;
}

void LiveViewSearch::initializeData()
{
    ui->deviceSearch->initializeData();
}

void LiveViewSearch::dealMessage(MessageReceive *message)
{
    if (message->isAccepted()) {
        return;
    }
    if (!isVisible()) {
        return;
    }
    ui->deviceSearch->dealMessage(message);
}

void LiveViewSearch::onLanguageChanged()
{
    ui->label_title->setText(GET_TEXT("CHANNELMANAGE/30002", "Device Search"));
}

#include "AnprItemMenu.h"
#include "MsLanguage.h"

AnprItemMenu::AnprItemMenu(QWidget *parent) :
    QMenu(parent)
{
    m_labelTitle = new QLabel(this);
    m_labelTitle->setMinimumHeight(30);
    m_labelTitle->setAlignment(Qt::AlignCenter);

    m_menuTitle = new QWidgetAction(this);
    m_menuTitle->setDefaultWidget(m_labelTitle);
    addAction(m_menuTitle);
    addSeparator();
    m_menuList = new QMenu(this);
    addMenu(m_menuList);
    m_actionBlack = new QAction(this);
    connect(m_actionBlack, SIGNAL(triggered(bool)), this, SLOT(onActionBlackClicked()));
    m_menuList->addAction(m_actionBlack);
    m_actionWhite = new QAction(this);
    connect(m_actionWhite, SIGNAL(triggered(bool)), this, SLOT(onActionWhiteClicked()));
    m_menuList->addAction(m_actionWhite);
    addSeparator();
    m_actionRemove = new QAction(this);
    connect(m_actionRemove, SIGNAL(triggered(bool)), this, SLOT(onActionRemoveClicked()));
    addAction(m_actionRemove);

    connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
    onLanguageChanged();
}

AnprItemMenu::~AnprItemMenu()
{

}

void AnprItemMenu::setPlate(const QString &plate)
{
    m_strPlate = plate;
    m_labelTitle->setText(QString("%1: %2").arg(GET_TEXT("ANPR/103004", "License")).arg(plate));
}

void AnprItemMenu::onLanguageChanged()
{
    m_menuList->setTitle(GET_TEXT("ANPR/103066", "Add to B/W List"));
    m_actionBlack->setText(GET_TEXT("ANPR/103067", "Add to Black List"));
    m_actionWhite->setText(GET_TEXT("ANPR/103068", "Add to White List"));
    m_actionRemove->setText(GET_TEXT("ANPR/103069", "Delete from B/W List"));
}

void AnprItemMenu::onActionBlackClicked()
{
    emit addBlacklist(m_strPlate);
}

void AnprItemMenu::onActionWhiteClicked()
{
    emit addWhitelist(m_strPlate);
}

void AnprItemMenu::onActionRemoveClicked()
{
    emit removeFromList(m_strPlate);
}


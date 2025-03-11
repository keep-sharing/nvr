#include "IrregularLayout.h"
#include "ui_IrregularLayout.h"
#include "CustomLayoutData.h"
#include "LiveView.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MultiScreenControl.h"
#include "MyDebug.h"
#include <QMouseEvent>

IrregularLayout *IrregularLayout::self = nullptr;

IrregularLayout::IrregularLayout(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::IrregularLayout)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    self = this;

    m_layoutButtonGroup = new QButtonGroup(this);
    m_layoutButtonGroup->setExclusive(false);
    m_layoutButtonGroup->addButton(ui->toolButton_layout8, LAYOUTMODE_8);
    m_layoutButtonGroup->addButton(ui->toolButton_layout8_1, LAYOUTMODE_8_1);
    m_layoutButtonGroup->addButton(ui->toolButton_layout12_1, LAYOUTMODE_12_1);
    m_layoutButtonGroup->addButton(ui->toolButton_layout14, LAYOUTMODE_14);
    m_layoutButtonGroup->addButton(ui->toolButton_layout32_2, LAYOUTMODE_32_2);
    connect(m_layoutButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(onLayoutButtonGroupClicked(int)));

    ui->toolButton_layout8->setNormalIcon(":/bottombar/bottombar/layout_4+4.png");
    ui->toolButton_layout8->setCheckedIcon(":/bottombar/bottombar/layout_4+4_checked.png");
    ui->toolButton_layout8->setData("LAYOUTMODE_8", Qt::UserRole);
    ui->toolButton_layout8_1->setNormalIcon(":/bottombar/bottombar/layout_1+7.png");
    ui->toolButton_layout8_1->setCheckedIcon(":/bottombar/bottombar/layout_1+7_checked.png");
    ui->toolButton_layout8_1->setData("LAYOUTMODE_8_1", Qt::UserRole);
    ui->toolButton_layout12_1->setNormalIcon(":/bottombar/bottombar/layout_1+11.png");
    ui->toolButton_layout12_1->setCheckedIcon(":/bottombar/bottombar/layout_1+11_checked.png");
    ui->toolButton_layout12_1->setData("LAYOUTMODE_12_1", Qt::UserRole);
    ui->toolButton_layout14->setNormalIcon(":/bottombar/bottombar/layout_2+12.png");
    ui->toolButton_layout14->setCheckedIcon(":/bottombar/bottombar/layout_2+12_checked.png");
    ui->toolButton_layout14->setData("LAYOUTMODE_14", Qt::UserRole);
    ui->toolButton_layout32_2->setNormalIcon(":/bottombar/bottombar/layout_1+3+28.png");
    ui->toolButton_layout32_2->setCheckedIcon(":/bottombar/bottombar/layout_1+3+28_checked.png");
    ui->toolButton_layout32_2->setData("LAYOUTMODE_32_2", Qt::UserRole);

    //屏蔽大于机型通道数的布局
    int leftHeight = 350;
    int dHeight = 61;
    int channelCount = qMsNvr->maxChannel();
    if (channelCount < 32) {
        ui->toolButton_layout32_2->hide();
        leftHeight -= dHeight;
    }
    if (channelCount < 14) {
        ui->toolButton_layout14->hide();
        leftHeight -= dHeight;
    }
    if (channelCount < 12) {
        ui->toolButton_layout12_1->hide();
        leftHeight -= dHeight;
    }
    if (channelCount < 8) {
        ui->toolButton_layout8_1->hide();
        leftHeight -= dHeight;
        ui->toolButton_layout8->hide();
        leftHeight -= dHeight;
    }
    ui->widgetLeft->resize(ui->widgetLeft->width(), leftHeight);

    ui->toolButton_layout8->installEventFilter(this);
    ui->toolButton_layout8_1->installEventFilter(this);
    ui->toolButton_layout12_1->installEventFilter(this);
    ui->toolButton_layout14->installEventFilter(this);
    ui->toolButton_layout32_2->installEventFilter(this);
    ui->toolButtonCustomLayout->installEventFilter(this);

    ui->listWidgetCustom->hide();
}

IrregularLayout::~IrregularLayout()
{
    self = nullptr;
    delete ui;
}

IrregularLayout *IrregularLayout::instance()
{
    return self;
}

void IrregularLayout::setLayout14Visible(bool visible)
{
    ui->toolButton_layout14->setVisible(visible);
}

void IrregularLayout::setCurrentLayoutButton(const CustomLayoutKey &key)
{
    QList<QAbstractButton *> buttonList = m_layoutButtonGroup->buttons();
    for (int i = 0; i < buttonList.size(); ++i) {
        MyIconToolButton *button = static_cast<MyIconToolButton *>(buttonList.at(i));
        button->setChecked(false);
    }
    //
    initializeCustomLayoutMenu();
    //
    switch (key.type()) {
    case CustomLayoutKey::DefaultType: {
        for (int i = 0; i < buttonList.size(); ++i) {
            MyIconToolButton *button = static_cast<MyIconToolButton *>(buttonList.at(i));
            const QString &text = button->data(Qt::UserRole).toString();
            if (text == key.name()) {
                button->setChecked(true);
                break;
            }
        }
        break;
    }
    case CustomLayoutKey::CustomType:
        break;
    default:
        break;
    }
}

void IrregularLayout::onLanguageChanged()
{
    ui->toolButton_layout8->setToolTip(GET_TEXT("LIVEVIEW/20003", "8 Screen"));
    ui->toolButton_layout8_1->setToolTip(GET_TEXT("LIVEVIEW/20004", "1+7 Screen"));
    ui->toolButton_layout12_1->setToolTip(GET_TEXT("LIVEVIEW/20007", "1+11 Screen"));
    ui->toolButton_layout14->setToolTip(GET_TEXT("LIVEVIEW/20008", "14 Screen"));
    ui->toolButton_layout32_2->setToolTip(GET_TEXT("LIVEVIEW/20030", "1+31 Screen"));
}

bool IrregularLayout::eventFilter(QObject *obj, QEvent *event)
{
    switch (event->type()) {
    case QEvent::Enter:
        if (obj == ui->toolButtonCustomLayout) {
            showCustomLayoutMenu();
        } else {
            hideCustomLayoutMenu();
        }
        break;
    case QEvent::Leave:
        break;
    default:
        break;
    }
    return QWidget::eventFilter(obj, event);
}

void IrregularLayout::showEvent(QShowEvent *event)
{
    adjustWidget();
    QWidget::showEvent(event);
}

void IrregularLayout::hideEvent(QHideEvent *event)
{
    hideCustomLayoutMenu();
    QWidget::hideEvent(event);
}

void IrregularLayout::resizeEvent(QResizeEvent *event)
{
    adjustWidget();
    QWidget::resizeEvent(event);
}

void IrregularLayout::mousePressEvent(QMouseEvent *event)
{
    QPoint p = event->pos();
    if (ui->widgetLeft->geometry().contains(p)) {

    } else if (ui->widgetRight->isVisible() && ui->widgetRight->geometry().contains(p)) {

    } else {
        hide();
    }
    return QWidget::mousePressEvent(event);
}

void IrregularLayout::adjustWidget()
{
    ui->widgetLeft->move(6, height() - 6 - ui->widgetLeft->height());
    ui->widgetRight->move(6 + ui->widgetLeft->width() - 1, height() - 6 - ui->widgetRight->height());
}

void IrregularLayout::initializeCustomLayoutMenu()
{
    const CustomLayoutKey &key = LiveView::instance()->currentLayoutMode();
    QListWidgetItem *itemSelected = nullptr;

    int screen = gMultiScreenControl.currentScreen();
    QStringList names = CustomLayoutData::instance()->customLayoutNames(screen);

    //5016机型，target模式下屏蔽通道数大于12的布局
    QStringList realNames = names;
    if (qMsNvr->is5016()) {
        if (LiveView::instance()->isTargetMode()) {
            for (int i = 0; i < names.size(); ++i) {
                const QString &name = names.at(i);
                CustomLayoutKey key(name, screen, CustomLayoutKey::CustomType);
                const CustomLayoutInfo &info = CustomLayoutData::instance()->layoutInfo(key);
                if (info.isValid())
                {
                    if (info.positionCount() > 12) {
                        realNames.removeAll(name);
                    }
                } else {
                    qMsWarning() << "invalid key:" << key;
                }
            }
        }
    }

    QFontMetrics fm(ui->listWidgetCustom->font());
    int maxLength = fm.width("Custom Layout 10");
    ui->listWidgetCustom->clear();
    for (int i = 0; i < realNames.size(); ++i) {
        const QString &name = realNames.at(i);
        const QString &elidedText = fm.elidedText(name, Qt::ElideRight, maxLength);
        QListWidgetItem *item = new QListWidgetItem(elidedText);
        item->setData(RealCustomLayoutNameRole, name);
        item->setToolTip(name);
        ui->listWidgetCustom->addItem(item);
        //
        if (key.type() == CustomLayoutKey::CustomType && key.name() == name) {
            itemSelected = item;
        }
    }
    if (realNames.isEmpty()) {
        m_isCustomEnable = false;
    } else {
        m_isCustomEnable = true;
    }

    ui->widgetRight->resize(ui->widgetRight->width(), realNames.size() * 30 + 2);

    if (m_isCustomEnable) {
        ui->toolButtonCustomLayout->setEnabled(true);
        if (itemSelected) {
            ui->listWidgetCustom->setCurrentItem(itemSelected);
            ui->toolButtonCustomLayout->setIcon(QIcon(":/layout/layout/custom_layout_blue.png"));
        } else {
            ui->toolButtonCustomLayout->setIcon(QIcon(":/layout/layout/custom_layout_white.png"));
        }
    } else {
        ui->toolButtonCustomLayout->setEnabled(false);
        ui->toolButtonCustomLayout->setIcon(QIcon(":/layout/layout/custom_layout_gray.png"));
    }
}

void IrregularLayout::showCustomLayoutMenu()
{
    if (!m_isCustomEnable) {
        return;
    }
    ui->listWidgetCustom->show();
}

void IrregularLayout::hideCustomLayoutMenu()
{
    ui->listWidgetCustom->hide();
}

void IrregularLayout::on_listWidgetCustom_itemClicked(QListWidgetItem *item)
{
    QString name = item->data(RealCustomLayoutNameRole).toString();
    emit customLayoutClicked(name);
}

void IrregularLayout::onLayoutButtonGroupClicked(int id)
{
    emit layoutButtonClicked(id);
}

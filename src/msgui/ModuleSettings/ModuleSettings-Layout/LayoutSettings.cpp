#include "LayoutSettings.h"
#include "ui_LayoutSettings.h"
#include "CustomLayoutData.h"
#include "CustomLayoutDialog.h"
#include "CustomLayoutPanel.h"
#include "LiveView.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MultiScreenControl.h"
#include "MyButtonGroup.h"
#include "MyDebug.h"
#include "PageLayout.h"
#include "mypushbutton.h"
#include <qmath.h>

LayoutSettings *LayoutSettings::s_customLayout = nullptr;

LayoutSettings::LayoutSettings(QWidget *parent)
    : AbstractSettingPage(parent)
    , ui(new Ui::LayoutSettings)
{
    ui->setupUi(this);
    s_customLayout = this;

    m_layoutButtonGroup = new MyButtonGroup(this);
    m_layoutButtonGroup->setExclusive(false);
    m_layoutButtonGroup->addButton(ui->toolButton_layout1, LAYOUTMODE_1);
    m_layoutButtonGroup->addButton(ui->toolButton_layout4, LAYOUTMODE_4);
    m_layoutButtonGroup->addButton(ui->toolButton_layout8, LAYOUTMODE_8);
    m_layoutButtonGroup->addButton(ui->toolButton_layout8_1, LAYOUTMODE_8_1);
    m_layoutButtonGroup->addButton(ui->toolButton_layout9, LAYOUTMODE_9);
    m_layoutButtonGroup->addButton(ui->toolButton_layout12, LAYOUTMODE_12);
    m_layoutButtonGroup->addButton(ui->toolButton_layout12_1, LAYOUTMODE_12_1);
    m_layoutButtonGroup->addButton(ui->toolButton_layout14, LAYOUTMODE_14);
    m_layoutButtonGroup->addButton(ui->toolButton_layout16, LAYOUTMODE_16);
    m_layoutButtonGroup->addButton(ui->toolButton_layout25, LAYOUTMODE_25);
    m_layoutButtonGroup->addButton(ui->toolButton_layout32, LAYOUTMODE_32);
    m_layoutButtonGroup->addButton(ui->toolButton_layout32_2, LAYOUTMODE_32_2);
    m_layoutButtonGroup->addButton(ui->toolButton_layout64, LAYOUTMODE_64);
    connect(m_layoutButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(onDefaultLayoutClicked(int)));

    ui->toolButtonCustom->setTrianglePosition(CustomLayoutButton::BottomRight);

    m_modeButtons.insert("LAYOUTMODE_1", ui->toolButton_layout1);
    m_modeButtons.insert("LAYOUTMODE_4", ui->toolButton_layout4);
    m_modeButtons.insert("LAYOUTMODE_8", ui->toolButton_layout8);
    m_modeButtons.insert("LAYOUTMODE_8_1", ui->toolButton_layout8_1);
    m_modeButtons.insert("LAYOUTMODE_9", ui->toolButton_layout9);
    m_modeButtons.insert("LAYOUTMODE_12", ui->toolButton_layout12);
    m_modeButtons.insert("LAYOUTMODE_12_1", ui->toolButton_layout12_1);
    m_modeButtons.insert("LAYOUTMODE_14", ui->toolButton_layout14);
    m_modeButtons.insert("LAYOUTMODE_16", ui->toolButton_layout16);
    m_modeButtons.insert("LAYOUTMODE_25", ui->toolButton_layout25);
    m_modeButtons.insert("LAYOUTMODE_32", ui->toolButton_layout32);
    m_modeButtons.insert("LAYOUTMODE_32_2", ui->toolButton_layout32_2);
    m_modeButtons.insert("LAYOUTMODE_64", ui->toolButton_layout64);
    m_modeButtons.insert("Custom", ui->toolButtonCustom);

    m_channelButtonGroup = new MyButtonGroup(this);
    connect(m_channelButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(onChannelButtonClicked(int)));
    int row = 0;
    int column = 0;
    for (int i = 0; i < qMsNvr->maxChannel(); ++i) {
        MyPushButton *button = new MyPushButton(this);
        button->setText(QString::number(i + 1));
        button->setIcon(QIcon(":/layout/layout/camera_black.png"));
        button->setCheckable(true);
        m_channelButtonGroup->addButton(button, i);
        if (column > 4) {
            column = 0;
            row++;
        }
        ui->gridLayout_channel->addWidget(button, row, column);
        column++;
    }

    //屏蔽大于机型通道数的布局
    int channelCount = qMsNvr->maxChannel();
    if (channelCount < 64) {
        ui->toolButton_layout64->hide();
    }
    if (channelCount < 32) {
        ui->toolButton_layout32_2->hide();
        ui->toolButton_layout32->hide();
    }
    if (channelCount < 25) {
        ui->toolButton_layout25->hide();
    }
    if (channelCount < 16) {
        ui->toolButton_layout16->hide();
    }
    if (channelCount < 14) {
        ui->toolButton_layout14->hide();
    }
    if (channelCount < 12) {
        ui->toolButton_layout12_1->hide();
        ui->toolButton_layout12->hide();
    }
    if (channelCount < 8) {
        ui->toolButton_layout8_1->hide();
        ui->toolButton_layout8->hide();
        ui->toolButton_layout9->hide();
    }

    connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
    onLanguageChanged();
}

LayoutSettings::~LayoutSettings()
{
    s_customLayout = nullptr;
    delete ui;
}

LayoutSettings *LayoutSettings::instance()
{
    return s_customLayout;
}

int LayoutSettings::channelFromGlobalIndex(const CustomLayoutKey &key, int index)
{
    for (int i = 0; i < m_allLayouts.size(); ++i) {
        const auto &info = m_allLayouts.at(i);
        if (info.key() == key) {
            return info.channel(index);
        }
    }
    return -1;
}

void LayoutSettings::setChannelFromGlobalIndex(const CustomLayoutKey &key, int index, int channel)
{
    CustomLayoutInfo &info = m_allLayouts.find(key);
    if (info.isValid()) {
        info.updateChannel(index, channel);
    } else {
        qMsWarning() << "invalid key:" << key;
    }

    //
    for (int i = 0; i < m_pageList.size(); ++i) {
        PageLayout *page = m_pageList.at(i);
        page->updateChannelPreview();
    }
    if (m_currentPage) {
        m_currentPage->updateChannel();
    }
}

void LayoutSettings::initializeData()
{
    m_channelButtonGroup->setCurrentId(0);
    //
    int multiSupport = gMultiScreenControl.multiScreenSupport();
    switch (multiSupport) {
    case 0:
        m_currentScreen = SCREEN_MAIN;
        ui->widgetScreen->hide();
        break;
    case 1:
        m_currentScreen = SCREEN_MAIN;
        ui->widgetScreen->show();
        ui->comboBoxScreen->beginEdit();
        ui->comboBoxScreen->clear();
        ui->comboBoxScreen->addItem("HDMI1/VGA1", SCREEN_MAIN);
        ui->comboBoxScreen->addItem("HDMI2/VGA2", SCREEN_SUB);
        ui->comboBoxScreen->endEdit();
        break;
    case 2:
    case 3:
        m_currentScreen = SCREEN_MAIN;
        ui->widgetScreen->show();
        ui->comboBoxScreen->beginEdit();
        ui->comboBoxScreen->clear();
        ui->comboBoxScreen->addItem("HDMI", SCREEN_MAIN);
        ui->comboBoxScreen->addItem("VGA", SCREEN_SUB);
        ui->comboBoxScreen->endEdit();
        break;
    default:
        break;
    }
    //
    m_allLayouts = CustomLayoutData::instance()->allLayouts();
    //
    CustomLayoutKey key = LiveView::instance()->currentLayoutMode(m_currentScreen);
    bool hasKey = false;
    for (int i = 0; i < m_allLayouts.size(); ++i) {
        const auto &info = m_allLayouts.at(i);
        if (info.key() == key) {
            hasKey = true;
            break;
        }
    }
    if (!hasKey) {
        key = CustomLayoutKey("LAYOUTMODE_4", m_currentScreen, CustomLayoutKey::DefaultType);
    }
    QMetaObject::invokeMethod(this, "onLayoutModeClicked", Qt::QueuedConnection, Q_ARG(CustomLayoutKey, key));
}

void LayoutSettings::processMessage(MessageReceive *message)
{
    Q_UNUSED(message)
}

void LayoutSettings::updateLayoutModeButtonState()
{
    for (auto iter = m_modeButtons.constBegin(); iter != m_modeButtons.constEnd(); ++iter) {
        const QString &mode = iter.key();
        QToolButton *button = iter.value();
        if (m_currentKey.type() == CustomLayoutKey::DefaultType && m_currentKey.name() == mode) {
            button->setChecked(true);
        } else if (m_currentKey.type() == CustomLayoutKey::CustomType && mode == QString("Custom")) {
            button->setChecked(true);
        } else {
            button->setChecked(false);
        }
    }
}

void LayoutSettings::showPreviewLayouts(const CustomLayoutKey &key)
{
    for (int i = 0; i < m_pageList.size(); ++i) {
        PageLayout *layout = m_pageList.at(i);
        layout->deleteLater();
    }
    m_pageList.clear();

    //
    const auto &layoutInfo = m_allLayouts.find(key);
    if (!layoutInfo.isValid()) {
        qMsWarning() << "invalid key:" << key;
        return;
    }
    int positionCount = layoutInfo.positionCount();
    int pageCount = layoutInfo.pageCount();
    for (int i = 0; i < pageCount; ++i) {
        PageLayout *layout = new PageLayout(PageLayout::ModePreview, ui->widget_preview);
        connect(layout, SIGNAL(clicked(CustomLayoutKey, int)), this, SLOT(onLayoutPageClicked(CustomLayoutKey, int)));
        layout->initializeData(layoutInfo, i);
        m_pageList.append(layout);
    }

    //显示预览缩略图
    int rowCount = 0;
    int columnCount = 0;
    if (positionCount < 4) {
        rowCount = qCeil(pageCount / 8.0);
        columnCount = 8;
    } else {
        rowCount = qCeil(pageCount / 4.0);
        columnCount = 4;
    }
    if (pageCount < columnCount) {
        columnCount = pageCount;
    }

    //取一块16:9的区域
    int w = 0;
    int h = 0;
    qreal ratio = (16.0 * columnCount) / (9.0 * rowCount);
    int contentWidth = ui->widget_preview->width() - 20;
    int contentHeight = ui->widget_preview->height() - 20;
    if (qreal(contentWidth) / contentHeight > ratio) {
        w = contentHeight * ratio;
        h = contentHeight;
    } else {
        w = contentWidth;
        h = contentWidth / ratio;
    }
    QRect rc = QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, QSize(w, h), ui->widget_preview->rect());
    int pageWidth = (w - (columnCount - 1) * 10) / columnCount;
    int pageHeight = (h - (rowCount - 1) * 10) / rowCount;
    int row = 0;
    int column = 0;
    for (int i = 0; i < m_pageList.size(); ++i) {
        int left = rc.left() + column * pageWidth + (column - 1) * 10;
        int top = rc.top() + row * pageHeight + (row - 1) * 10;
        PageLayout *layout = m_pageList.at(i);
        layout->setGeometry(QRect(left, top, pageWidth, pageHeight));
        layout->show();
        column++;
        if (column >= columnCount) {
            column = 0;
            row++;
        }
    }
}

void LayoutSettings::onLanguageChanged()
{
    ui->labelScreen->setText(GET_TEXT("LAYOUT/40008", "Screen Output"));
    ui->pushButton_reset->setText(GET_TEXT("COMMON/1057", "Reset"));
    ui->pushButton_apply->setText(GET_TEXT("COMMON/1003", "Apply"));
    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));
}

void LayoutSettings::on_comboBoxScreen_indexSet(int index)
{
    m_currentScreen = ui->comboBoxScreen->itemData(index).toInt();
    bool hide64Layout = qMsNvr->maxChannel() < 64 || (qMsNvr->is3536g() && SCREEN_MAIN != m_currentScreen);
    ui->toolButton_layout64->setVisible(!hide64Layout);

    CustomLayoutKey key = LiveView::instance()->currentLayoutMode(m_currentScreen);
    if (!key.isValid()) {
        key = CustomLayoutKey("LAYOUTMODE_4", m_currentScreen, CustomLayoutKey::DefaultType);
    }
    onLayoutModeClicked(key);
}

void LayoutSettings::onChannelButtonClicked(int channel)
{
    ui->video->playVideo(channel);

    if (m_currentPage) {
        m_currentPage->setChannel(channel);
    }
}

void LayoutSettings::onDefaultLayoutClicked(int id)
{
    QString name = CustomLayoutData::instance()->nameFromDefaultLayoutMode(id);
    onLayoutModeClicked(CustomLayoutKey(name, m_currentScreen, CustomLayoutKey::DefaultType));
}

void LayoutSettings::onCustomLayoutClicked(const QString &name)
{
    onLayoutModeClicked(CustomLayoutKey(name, m_currentScreen, CustomLayoutKey::CustomType));
}

void LayoutSettings::onLayoutModeClicked(const CustomLayoutKey &key)
{
    m_currentKey = key;
    updateLayoutModeButtonState();
    showPreviewLayouts(m_currentKey);
    onLayoutPageClicked(m_currentKey, 0);
}

void LayoutSettings::onLayoutPageClicked(const CustomLayoutKey &key, int page)
{
    qMsDebug() << key << "page:" << page;
    ui->label_page->setText(QString("%1%2").arg(GET_TEXT("LAYOUT/40009", "Page: ")).arg(page + 1));
    //
    for (int i = 0; i < m_pageList.size(); ++i) {
        PageLayout *layout = m_pageList.at(i);
        if (layout->page() == page) {
            layout->setChecked(true);
        } else {
            layout->setChecked(false);
        }
    }
    //
    if (m_currentPage) {
        ui->gridLayout_currentPage->removeWidget(m_currentPage);
        m_currentPage->deleteLater();
        m_currentPage = nullptr;
    }
    m_currentPage = new PageLayout(PageLayout::ModeDetail, this);
    m_currentPage->initializeData(m_allLayouts.find(key), page);
    ui->gridLayout_currentPage->addWidget(m_currentPage);
}

void LayoutSettings::on_toolButtonCustom_clicked()
{
    ui->toolButtonCustom->setChecked(false);

    if (!m_customPanel) {
        m_customPanel = new CustomLayoutPanel(this);
        connect(m_customPanel, SIGNAL(itemClicked(QString)), this, SLOT(onCustomLayoutClicked(QString)));
        connect(m_customPanel, SIGNAL(settingClicked()), this, SLOT(onCustomLayoutSettingClicked()));
        connect(m_customPanel, SIGNAL(closed()), this, SLOT(onCustomLayoutPanelClosed()));
    }
    m_customPanel->show();
    QPoint p = ui->toolButtonCustom->mapToGlobal(QPoint(0, 0));
    p.setX(p.x() + ui->toolButtonCustom->width() / 2 - m_customPanel->width() / 2 - 5);
    p.setY(p.y() + ui->toolButtonCustom->height() / 2 + 20);
    m_customPanel->move(p);
    m_customPanel->initializeData(m_currentScreen, m_currentKey, m_allLayouts.customLayoutNames(m_currentScreen));
}

void LayoutSettings::onCustomLayoutSettingClicked()
{
    if (!m_customLayoutDialog) {
        m_customLayoutDialog = new CustomLayoutDialog(this);
        connect(m_customLayoutDialog, SIGNAL(saved()), this, SLOT(onCustomLayoutSaved()));
        connect(m_customLayoutDialog, SIGNAL(closed()), this, SLOT(onCustomLayoutClosed()));
    }
    m_customLayoutDialog->show();
    m_customLayoutDialog->initializeData(m_currentScreen, m_allLayouts);
}

void LayoutSettings::onCustomLayoutPanelClosed()
{
    updateLayoutModeButtonState();
}

void LayoutSettings::onCustomLayoutSaved()
{
    m_allLayouts = m_customLayoutDialog->allLayouts();

    if (!m_allLayouts.contains(m_currentKey)) {
        m_currentKey = CustomLayoutKey("LAYOUTMODE_4", m_currentScreen, CustomLayoutKey::DefaultType);
    }
    onLayoutModeClicked(m_currentKey);
    updateLayoutModeButtonState();
}

void LayoutSettings::onCustomLayoutClosed()
{
    updateLayoutModeButtonState();
}

void LayoutSettings::on_pushButton_reset_clicked()
{
    ui->pushButton_reset->clearUnderMouse();

    const int result = MessageBox::question(this, GET_TEXT("LAYOUT/40006", "Are you sure to reset layout configuration?"));
    if (result == MessageBox::Cancel) {
        return;
    }

    m_allLayouts.resetChannels();

    //
    for (int i = 0; i < m_pageList.size(); ++i) {
        PageLayout *page = m_pageList.at(i);
        page->updateChannelPreview();
    }
    if (m_currentPage) {
        m_currentPage->updateChannel();
    }
}

void LayoutSettings::on_pushButton_apply_clicked()
{
    const int result = MessageBox::question(this, GET_TEXT("LAYOUT/40002", "Apply successfully. Copy settings of current layout to others’?"), GET_TEXT("PROFILE/76016", "Yes"), GET_TEXT("COMMON/1056", "No"));
    if (result == MessageBox::Yes) {
        qDebug() << "CustomLayout::on_pushButton_apply_clicked, save with copy.";
        const auto &currentInfo = m_allLayouts.find(m_currentKey);
        for (int i = 0; i < m_allLayouts.size(); ++i) {
            const auto &info = m_allLayouts.at(i);
            const auto &key = info.key();
            if (key != m_currentKey && key.screen() == m_currentScreen) {
                auto &value = m_allLayouts[i];
                value.setChannels(currentInfo.channels());
            }
        }
    } else if (result == MessageBox::Cancel) {
        qDebug() << "CustomLayout::on_pushButton_apply_clicked, save only.";
    }

    //
    CustomLayoutData::instance()->setAllLayouts(m_allLayouts);
    CustomLayoutData::instance()->saveAllLayouts();
    CustomLayoutData::instance()->saveOldLayouts();

    //
    LiveView::instance()->resetSubLayoutMode();
}

void LayoutSettings::on_pushButton_back_clicked()
{
    emit sig_back();
}

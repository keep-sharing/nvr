#include "OccupancyGroupSettings.h"
#include "ui_OccupancyGroupSettings.h"
#include "MyDebug.h"
#include "centralmessage.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MsWaitting.h"
#include "OccupancyGroupEdit.h"
#include "PeopleCountingData.h"


OccupancyGroupSettings::OccupancyGroupSettings(QWidget *parent)
    : BaseShadowDialog(parent)
    , ui(new Ui::OccupancyGroupSettings)
{
    ui->setupUi(this);

    //
    QStringList headers;
    headers << "";
    headers << GET_TEXT("OCCUPANCY/74251", "Group No.");
    headers << GET_TEXT("OCCUPANCY/74217", "Group Name");
    headers << GET_TEXT("CHANNELMANAGE/30008", "Channel");
    headers << GET_TEXT("COMMON/1019", "Edit");
    headers << GET_TEXT("CHANNELMANAGE/30023", "Delete");
    ui->tableView->setHorizontalHeaderLabels(headers);
    ui->tableView->setHeaderCheckable(false);
    ui->tableView->setColumnCount(headers.size());
    ui->tableView->setSortingEnabled(false);
    ui->tableView->hideColumn(ColumnCheck);
    //delegate
    ui->tableView->setItemDelegateForColumn(ColumnGroup, new ItemIconDelegate(this));
    ui->tableView->setItemDelegateForColumn(ColumnEdit, new ItemButtonDelegate(QPixmap(":/common/common/edit.png"), this));
    ui->tableView->setItemDelegateForColumn(ColumnDelete, new ItemButtonDelegate(QPixmap(":/common/common/delete.png"), this));
    //
    ui->tableView->setColumnWidth(ColumnGroup, 150);
    ui->tableView->setColumnWidth(ColumnName, 150);
    ui->tableView->setColumnWidth(ColumnChannel, 400);
    ui->tableView->setColumnWidth(ColumnEdit, 150);
    ui->tableView->setMinimumHeight(MAX_PEOPLECNT_GROUP * 30 + 32);
    ui->tableView->setMaximumHeight(MAX_PEOPLECNT_GROUP * 30 + 32);
    //
    connect(ui->tableView, SIGNAL(itemClicked(int, int)), this, SLOT(onGroupTableClicked(int, int)));

    //
    m_groupEdit = new OccupancyGroupEdit(this);

    //
    onLanguageChanged();
}

OccupancyGroupSettings::~OccupancyGroupSettings()
{
    if (m_cacheData) {
        delete[] m_cacheData;
        m_cacheData = nullptr;
    }

    delete ui;
}

void OccupancyGroupSettings::setSourceData(PEOPLECNT_SETTING *source_array)
{
    m_sourceData = source_array;
    if (!m_cacheData) {
        m_cacheData = new PEOPLECNT_SETTING[MAX_PEOPLECNT_GROUP];
    }
    memset(m_cacheData, 0, sizeof(PEOPLECNT_SETTING) * MAX_PEOPLECNT_GROUP);
    memcpy(m_cacheData, m_sourceData, sizeof(PEOPLECNT_SETTING) * MAX_PEOPLECNT_GROUP);

    showGroupTable();
}

void OccupancyGroupSettings::showGroupTable()
{
    ui->tableView->clearContent();
    m_rowCount = 0;
    for (int i = 0; i < MAX_PEOPLECNT_GROUP; ++i) {
        const PEOPLECNT_SETTING &setting = m_cacheData[i];
        if (PeopleCountingData::hasChannel(setting.tri_channels)) {
            ui->tableView->setItemIntValue(m_rowCount, ColumnGroup, i + 1);
            ui->tableView->setItemText(m_rowCount, ColumnName, setting.name);
            ui->tableView->setItemToolTip(m_rowCount, ColumnName, setting.name);
            QString channelText = channelString(setting.tri_channels);
            ui->tableView->setItemText(m_rowCount, ColumnChannel, channelText);
            ui->tableView->setItemToolTip(m_rowCount, ColumnChannel, channelText);
            m_rowCount++;
        }
    }
    if (m_rowCount < MAX_PEOPLECNT_GROUP) {
        ui->tableView->setItemPixmap(m_rowCount, ColumnGroup, QPixmap(":/ptz/ptz/add.png"));
        ui->tableView->setItemText(m_rowCount, ColumnName, "-");
        ui->tableView->setItemText(m_rowCount, ColumnChannel, "-");
        ui->tableView->setItemText(m_rowCount, ColumnEdit, "-");
        ui->tableView->setItemText(m_rowCount, ColumnDelete, "-");
    }
}

QString OccupancyGroupSettings::channelString(const QString &text)
{
    QString result;
    for (int i = 0; i < text.size() && i < qMsNvr->maxChannel(); ++i) {
        if (text.at(i) == QChar('1')) {
            result.append(QString("%1").arg(i + 1));
            result.append(QString(", "));
        }
    }
    if (!result.isEmpty()) {
        result.chop(2);
    }
    return result;
}

void OccupancyGroupSettings::onLanguageChanged()
{
    ui->label_title->setText(GET_TEXT("OCCUPANCY/74216", "Group Settings"));
    ui->pushButtonOk->setText(GET_TEXT("COMMON/1001", "OK"));
    ui->pushButtonCancel->setText(GET_TEXT("COMMON/1004", "Cancel"));
}

void OccupancyGroupSettings::onGroupTableClicked(int row, int column)
{
    switch (column) {
    case ColumnGroup: {
        if (row == m_rowCount) {
            for (int i = 0; i < MAX_PEOPLECNT_GROUP; ++i) {
                PEOPLECNT_SETTING &setting = m_cacheData[i];
                if (!PeopleCountingData::hasChannel(setting.tri_channels)) {
                    int result = m_groupEdit->execAdd(&setting, i);
                    if (result == OccupancyGroupEdit::Accepted) {
                        showGroupTable();
                    }
                    break;
                }
            }
        }
        break;
    }
    case ColumnEdit: {
        if (row < m_rowCount) {
            int index = ui->tableView->itemIntValue(row, ColumnGroup) - 1;
            PEOPLECNT_SETTING &setting = m_cacheData[index];
            int result = m_groupEdit->execEdit(&setting, index);
            if (result == OccupancyGroupEdit::Accepted) {
                showGroupTable();
            }
        }
        break;
    }
    case ColumnDelete: {
        if (row < m_rowCount) {
            int index = ui->tableView->itemIntValue(row, ColumnGroup) - 1;
            PEOPLECNT_SETTING &setting = m_cacheData[index];
            memset(setting.tri_channels, 0, sizeof(setting.tri_channels));
            setting.stays = 99999;
            setting.enable = 0;
            setting.liveview_auto_reset = 0;
            setting.liveview_font_size = 1;
            setting.auto_day = 0;
            setting.liveview_display = -1;
            snprintf(setting.liveview_green_tips, sizeof(setting.liveview_green_tips), "%s", QString("Welcome!!!").toStdString().c_str());
            snprintf(setting.liveview_red_tips, sizeof(setting.liveview_red_tips), "%s", QString("Please wait till the green light turn on.").toStdString().c_str());
            snprintf(setting.name, sizeof(setting.name), "%s", QString("Group%1").arg(index + 1).toStdString().c_str());
            snprintf(setting.auto_day_time, sizeof(setting.auto_day_time), "%s", QString("00:00:00").toStdString().c_str());
            showGroupTable();
        }
        break;
    }
    default:
        break;
    }
}

void OccupancyGroupSettings::on_pushButtonOk_clicked()
{
    memcpy(m_sourceData, m_cacheData, sizeof(PEOPLECNT_SETTING) * MAX_PEOPLECNT_GROUP);
    accept();
}

void OccupancyGroupSettings::on_pushButtonCancel_clicked()
{
    reject();
}


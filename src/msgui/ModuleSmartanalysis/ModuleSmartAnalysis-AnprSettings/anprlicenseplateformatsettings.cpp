#include "anprlicenseplateformatsettings.h"
#include "ui_anprlicenseplateformatsettings.h"
#include "MsLanguage.h"
#include "anprlicenseplateformatsettings_p.h"
#include "licenseplateformateditor.h"
#include <QDebug>
#include <QPainter>
#include <QStandardItemModel>

extern "C" {
#include "msg.h"
}

AnprLicensePlateFormatSettingsPrivate::AnprLicensePlateFormatSettingsPrivate(QObject *parent)
    : QObject(parent)
{
    p = qobject_cast<AnprLicensePlateFormatSettings *>(parent);
    model = new AnprLicensePlateFormatSettingsModel(this);
    delegate = new AnprLicensePlateFormatSettingsDelegate(this);
    connect(p->ui->tableView, SIGNAL(clicked(const QModelIndex &)), model, SLOT(onItemClicked(const QModelIndex &)));
    connect(model, SIGNAL(openLicensePlateFormatEditor(bool)), this, SLOT(onOpenLicensePlateFormatEditor(bool)));
    connect(model, SIGNAL(rowsInserted(const QModelIndex &, int, int)), this, SLOT(onModelRowChanged(const QModelIndex &, int, int)));
    connect(model, SIGNAL(rowsRemoved(const QModelIndex &, int, int)), this, SLOT(onModelRowChanged(const QModelIndex &, int, int)));
    model->initialize();
}

void AnprLicensePlateFormatSettingsPrivate::onModelRowChanged(const QModelIndex &, int, int)
{
    if (model->rowCount() < Model::MinRow)
        return;

    bool noExtraId = true;

    if (model->rowCount() != Model::MinRow)
        noExtraId = false;

    if (model->rowCount() == Model::MaxRow) {
        for (int i = 0; i < model->columnCount(); i++)
            model->item(model->rowCount() - 1, i)->setData(true, Model::DisableRole);
    }
    model->item(model->rowCount() - 1, Model::Col_ID)->setData(model->rowCount() == Model::MaxRow ? QPixmap(":/common/common/add-disable.png") : QPixmap(":/ptz/ptz/add.png"), Qt::DecorationRole);

    p->ui->comboBox->setEnabled(!noExtraId);

    for (int i = 0; i < model->columnCount(); i++) {
        model->item(0, i)->setData(QVariant(!noExtraId), Model::DisableRole);
    }

    for (int i = 1; i < model->rowCount(); i++) {
        model->item(i, Model::Col_ID)->setData(QString::number(i), Qt::DisplayRole);
    }

    noExtraId = true;
    for (int i = 1; i < model->rowCount(); i++) {
        if (model->item(i, Model::Col_Enable)->index().sibling(i, Model::Col_Enable).data(Model::CheckRole).toBool()) {
            noExtraId = false;
        }
    }

    model->item(0, Model::Col_Enable)->setData(QVariant(noExtraId ? QPixmap(":/common/common/checkbox-checked-disable.png") : QPixmap(":/common/common/checkbox-unchecked-disable.png")), Qt::DecorationRole);
    model->item(0, Model::Col_Enable)->setData(noExtraId, Model::CheckRole);
}

void AnprLicensePlateFormatSettingsPrivate::onOpenLicensePlateFormatEditor(bool isAddingItem)
{
    LicensePlateFormatEditor editor(p, isAddingItem, wildcards->maxPlateCharNum);
    connect(&editor, SIGNAL(accepted()), this, SLOT(onLicensePlateFormatEditorAccepted()));
    if (!isAddingItem) {
        editor.setCharacterCount(model->item(model->selectedRow(), AnprLicensePlateFormatSettingsModel::Col_CharacterCount)->text().toInt());
        editor.setFormat(model->item(model->selectedRow(), AnprLicensePlateFormatSettingsModel::Col_Format)->text());
    }
    editor.exec();
}

void AnprLicensePlateFormatSettingsPrivate::populateModel()
{
    p->ui->comboBox->setCurrentIndex(wildcards->filter == 1 ? 0 : 1);
    for (int i = 0; i < 9; i++) {
        if (wildcards->cr_count[i] != 0) {
            model->addOrUpdateLine(i + 1, QString::number(wildcards->cr_count[i]), wildcards->format[i], wildcards->enable[i], true, true);
        }
    }
}

void AnprLicensePlateFormatSettingsPrivate::fetchFromModel()
{
    int channelId = wildcards->chanid;
    int maxNum = wildcards->maxPlateCharNum;
    memset(wildcards, '\0', sizeof(ms_lpr_wildcards));
    wildcards->chanid = channelId;
    wildcards->maxPlateCharNum = maxNum;
    for (int i = 0; i < 9 && i < model->rowCount() - 2; i++) {
        wildcards->enable[i] = model->item(i + 1, AnprLicensePlateFormatSettingsModel::Col_Enable)->data(AnprLicensePlateFormatSettingsModel::CheckRole).toBool();
        wildcards->cr_count[i] = model->item(i + 1, AnprLicensePlateFormatSettingsModel::Col_CharacterCount)->text().toInt();
        QString format = model->item(i + 1, AnprLicensePlateFormatSettingsModel::Col_Format)->text();
        memcpy(wildcards->format[i], format.toLocal8Bit(), format.size());
    }

    wildcards->filter = p->ui->comboBox->currentIndex() == 1 ? 0 : 1;
}

bool AnprLicensePlateFormatSettingsPrivate::isExistingCharacterCount(int count)
{
    for (int i = 1; i < model->rowCount(); i++) {
        if (count == model->item(i, AnprLicensePlateFormatSettingsModel::Col_CharacterCount)->text().toInt())
            return true;
    }
    return false;
}

void AnprLicensePlateFormatSettingsPrivate::onLicensePlateFormatEditorAccepted()
{
    auto editor = qobject_cast<LicensePlateFormatEditor *>(sender());
    if (editor->isAddingItem()) {
        model->addOrUpdateLine(model->rowCount() - 1, QString::number(editor->characterCount()), editor->format(), true, true, true);
    } else {
        model->addOrUpdateLine(model->selectedRow(), QString::number(editor->characterCount()), editor->format(),
                               model->item(model->selectedRow(), AnprLicensePlateFormatSettingsModel::Col_Enable)->data(AnprLicensePlateFormatSettingsModel::CheckRole).toBool(), true, true);
    }
}

void AnprLicensePlateFormatSettingsPrivate::translateUI()
{
    p->ui->pushButton_ok->setText(GET_TEXT("COMMON/1001", "OK"));
    p->ui->pushButton_cancel->setText(GET_TEXT("COMMON/1004", "Cancel"));
    p->ui->label_pushCharacterCount->setText(GET_TEXT("ANPR/103076", "Push Correct Character Count Results Only"));
    p->ui->label_formatExample->setText(GET_TEXT("ANPR/103077", "Format Example: AA111*"));
    p->ui->label_formatExampleLetters->setText(GET_TEXT("ANPR/103078", "A - Letters Only"));
    p->ui->label_formatExampleNumbers->setText(GET_TEXT("ANPR/103079", "1 - Numbers Only"));
    p->ui->label_formatExampleUnrestricted->setText(GET_TEXT("ANPR/103080", "* - Unrestricted Type"));
    p->ui->label_title->setText(GET_TEXT("ANPR/103081", "License Plate Format"));
}

AnprLicensePlateFormatSettingsModel::AnprLicensePlateFormatSettingsModel(QObject *parent)
    : QStandardItemModel(parent)
{
}

void AnprLicensePlateFormatSettingsModel::onItemClicked(const QModelIndex &index)
{
    if (m_selectedRow != -1)
        for (int i = 0; i < columnCount(); i++)
            item(m_selectedRow, i)->setData(false, SelectionRole);

    if (index.row() > 0 && index.row() < MaxRow - 1) {
        for (int i = 0; i < columnCount(); i++)
            item(index.row(), i)->setData(true, SelectionRole);
    }

    m_selectedRow = index.row();

    if (index.data(FunctionRole).isValid()) {
        switch (index.data(FunctionRole).toInt()) {
        case Function_AddLicense:
            if (rowCount() >= MaxRow)
                break;
            emit openLicensePlateFormatEditor(true);
            m_selectedRow = rowCount() - 1;
            break;
        case Function_EditLicense:
            emit openLicensePlateFormatEditor(false);
            break;
        case Function_DeleteLicense:
            removeRows(index.row(), 1);
            break;
        case Function_EnableLicense:
            enableLicense(index);
            break;
        }
    }
}

void AnprLicensePlateFormatSettingsModel::enableLicense(const QModelIndex &index)
{
    if (index.row() == 0)
        return;
    addOrUpdateLine(index.row(), index.sibling(index.row(), Col_CharacterCount).data(Qt::DisplayRole).toString(),
                    index.sibling(index.row(), Col_Format).data(Qt::DisplayRole).toString(), !index.sibling(index.row(), Col_Enable).data(CheckRole).toBool(),
                    index.row() == 0 ? false : true, index.row() == 0 ? false : true);
    bool allEnabled = true;
    for (int i = 1; i < rowCount(); i++) {
        if (index.sibling(i, Col_Enable).data(CheckRole).toBool())
            allEnabled = false;
    }
    item(0, Col_Enable)->setData(QVariant(allEnabled ? QPixmap(":/common/common/checkbox-checked-disable.png") : QPixmap(":/common/common/checkbox-unchecked-disable.png")), Qt::DecorationRole);
    item(0, Col_Enable)->setData(allEnabled, CheckRole);
}

void AnprLicensePlateFormatSettingsModel::loadHeaderData()
{
    for (int i = 0; i < headerNames.size(); i++)
        setHeaderData(i, Qt::Horizontal, GET_TEXT(headerNames[i].first, headerNames[i].second), Qt::DisplayRole);
}

int AnprLicensePlateFormatSettingsModel::selectedRow()
{
    return m_selectedRow;
}

void AnprLicensePlateFormatSettingsModel::addLastLine()
{
    QList<QStandardItem *> items;
    auto item = new QStandardItem;
    item->setData(QVariant(QPixmap(":/ptz/ptz/add.png")), Qt::DecorationRole);
    item->setData(QVariant(Function_AddLicense), FunctionRole);
    items << item;
    items << new QStandardItem("-");
    items << new QStandardItem("-");
    items << new QStandardItem("-");
    items << new QStandardItem("-");
    items << new QStandardItem("-");

    for (auto &i : items)
        i->setEditable(false);

    appendRow(items);
}

void AnprLicensePlateFormatSettingsModel::addOrUpdateLine(int id, const QString &count, const QString &format, bool enable, bool editable, bool deletable)
{
    QList<QStandardItem *> items;
    bool isNewLine = id >= rowCount() - 1;
    for (int i = 0; i < 6; i++) {
        QStandardItem *item = isNewLine ? new QStandardItem : this->item(id, i);
        item->setEditable(false);
        items << item;
    }
    items[Col_ID]->setText(QString::number(id));
    items[Col_CharacterCount]->setText(count);
    items[Col_Format]->setText(format);
    items[Col_Enable]->setData(QVariant(enable ? QPixmap(":/common/common/checkbox-checked.png") : QPixmap(":/common/common/checkbox-unchecked.png")), Qt::DecorationRole);
    items[Col_Enable]->setData(QVariant(enable), CheckRole);
    items[Col_Enable]->setData(QVariant(Function_EnableLicense), FunctionRole);
    items[Col_Edit]->setData(editable ? QVariant(QPixmap(":/common/common/edit.png")) : QVariant("-"), editable ? Qt::DecorationRole : Qt::DisplayRole);
    if (editable)
        items[Col_Edit]->setData(QVariant(Function_EditLicense), FunctionRole);
    items[Col_Delete]->setData(deletable ? QVariant(QPixmap(":/common/common/delete.png")) : QVariant("-"), deletable ? Qt::DecorationRole : Qt::DisplayRole);
    if (deletable)
        items[Col_Delete]->setData(QVariant(Function_DeleteLicense), FunctionRole);

    if (isNewLine)
        insertRow(rowCount() - 1, items);
}

void AnprLicensePlateFormatSettingsModel::initialize()
{
    addLastLine();
    addOrUpdateLine(0, "All", "*", true, false, false);
    loadHeaderData();
}

const QPair<QString, QString> &AnprLicensePlateFormatSettingsModel::headerName(int idx)
{
    return headerNames[idx];
}

QVector<QPair<QString, QString>> AnprLicensePlateFormatSettingsModel::headerNames = {
    { "ANPR/103082", "ID" },
    { "ANPR/103083", "License Plate Character Count" },
    { "ANPR/103081", "License Plate Format" },
    { "COMMON/1009", "Enable" },
    { "COMMON/1019", "Edit" },
    { "CHANNELMANAGE/30023", "Delete" }
};

AnprLicensePlateFormatSettingsDelegate::AnprLicensePlateFormatSettingsDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void AnprLicensePlateFormatSettingsDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
    QVariant selectionRole = index.data(AnprLicensePlateFormatSettingsModel::SelectionRole);
    if (selectionRole.isValid() && selectionRole.toBool()) {
        painter->fillRect(option.rect, Qt::gray);
    }

    QVariant disableRole = index.data(AnprLicensePlateFormatSettingsModel::DisableRole);
    if (disableRole.isValid() && disableRole.toBool()) {
        painter->setPen(QColor(Qt::gray));
    }

    if (index.data(Qt::DecorationRole).isValid()) {
        QRect rc = option.rect;
        QPixmap pixmap = index.data(Qt::DecorationRole).value<QPixmap>();
        if (index.column() == AnprLicensePlateFormatSettingsModel::Col_Enable)
            pixmap = pixmap.scaled(18, 18, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        else
            pixmap = pixmap.scaled(24, 24, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        rc.setSize(pixmap.size());
        rc.moveCenter(option.rect.center());
        painter->drawPixmap(rc, pixmap);
    } else if (index.data(Qt::DisplayRole).isValid()) {
        painter->drawText(option.rect, Qt::AlignHCenter | Qt::AlignVCenter, index.data(Qt::DisplayRole).toString());
    }
    painter->restore();
}

QSize AnprLicensePlateFormatSettingsDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QFontMetrics metrics(option.font);
    int width = metrics.width(Model::headerName(index.column()).second + "    ");
    int height = metrics.height();
    return QSize(width, height);
}

AnprLicensePlateFormatSettings::AnprLicensePlateFormatSettings(QWidget *parent, ms_lpr_wildcards *wildcards)
    : BaseShadowDialog(parent)
    , ui(new Ui::AnprLicensePlateFormatSettings)
{
    ui->setupUi(this);
    d = new AnprLicensePlateFormatSettingsPrivate(this);
    d->wildcards = wildcards;
    ui->tableView->setModel(d->model);
    ui->tableView->verticalHeader()->hide();
    auto itemDelegate = ui->tableView->itemDelegate();
    ui->tableView->setItemDelegate(d->delegate);
    delete itemDelegate;
    ui->tableView->resizeColumnsToContents();
    ui->tableView->horizontalHeader()->setStretchLastSection(true);
    ui->tableView->horizontalHeader()->setResizeMode(QHeaderView::Fixed);
    ui->tableView->horizontalHeader()->setClickable(false);
    ui->tableView->setSelectionMode(QAbstractItemView::NoSelection);
    ui->tableView->setAlternatingRowColors(true);
    ui->comboBox->addItem(GET_TEXT("COMMON/1009", "Enable"));
    ui->comboBox->addItem(GET_TEXT("COMMON/1018", "Disable"));
    d->populateModel();
    d->translateUI();
}

AnprLicensePlateFormatSettings::~AnprLicensePlateFormatSettings()
{
    delete ui;
}

void AnprLicensePlateFormatSettings::on_pushButton_cancel_clicked()
{
    reject();
}

void AnprLicensePlateFormatSettings::on_pushButton_ok_clicked()
{
    d->fetchFromModel();
    accept();
}

QObject *AnprLicensePlateFormatSettings::d_func()
{
    return d;
}

#include "patroleditdelegate.h"
#include "MsCameraVersion.h"
#include "combobox.h"
#include "ptzcontrol.h"
#include "ptzdatamanager.h"
#include <QSpinBox>

PatrolEditDelegate::PatrolEditDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

PatrolEditDelegate::~PatrolEditDelegate()
{
}

QWidget *PatrolEditDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)

    m_currentRow = index.row();

    switch (index.column()) {
    case 1: //预置点编号
    {
        ComboBox *comboBox = new ComboBox(parent);
        connect(comboBox, SIGNAL(activated(int)), this, SLOT(onCommitData()));
        //78版本及以上有300个预置点
        MsCameraVersion cameraVersion = MsCameraVersion::fromChannel(m_channel);
        int count = cameraVersion >= MsCameraVersion(7, 78) ? 300 : 255;
        //雨刷机型第53个点也为特殊预置点
        int limit = 52;
        gPtzDataManager->waitForGetSystemInfo();
        if (gPtzDataManager->isSupportWiper()) {
          limit = 53;
        }
        for (int i = 0; i < count; ++i) {
            //巡航预置点应屏蔽掉33-52之间的序号
            if (i > 31 && i < limit) {
                continue;
            }
            comboBox->addItem(QString("%1").arg(i + 1));
        }
        return comboBox;
    }
    case 2: //速度
    {
        ComboBox *comboBox = new ComboBox(parent);
        connect(comboBox, SIGNAL(activated(int)), this, SLOT(onCommitData()));
        for (int i = 0; i < 40; ++i) {
            comboBox->addItem(QString("%1").arg(i + 1));
        }
        return comboBox;
    }
    case 3: //时间
    {
        QSpinBox *spinBox = new QSpinBox(parent);
        connect(spinBox, SIGNAL(valueChanged(int)), this, SLOT(onCommitData()));
        //MSHN-6630 WEB&QT：高速球3.0巡航--TIME最短时间为10s
        //ptz bullet最小值:15，其他最小值10。
        if (gPtzDataManager->isPtzBullet()) {
            spinBox->setRange(15, 120);
        } else {
            spinBox->setRange(10, 120);
        }
        return spinBox;
    }
    default:
        return nullptr;
    }
}

void PatrolEditDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    switch (index.column()) {
    case 1: //预置点编号
    case 2: //速度
    {
        ComboBox *comboBox = qobject_cast<ComboBox *>(editor);
        int pos = comboBox->findText(index.model()->data(index).toString(), Qt::MatchExactly);
        comboBox->setCurrentIndex(pos);
        break;
    }
    case 3: //时间
    {
        QSpinBox *spinBox = qobject_cast<QSpinBox *>(editor);
        spinBox->setContextMenuPolicy(Qt::NoContextMenu);
        int value = index.model()->data(index).toInt();
        spinBox->setValue(value);
        break;
    }
    default:
        return QStyledItemDelegate::setEditorData(editor, index);
    }
}

void PatrolEditDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    switch (index.column()) {
    case 1: //预置点编号
    case 2: //速度
    {
        ComboBox *comboBox = qobject_cast<ComboBox *>(editor);
        model->setData(index, comboBox->currentText());
        break;
    }
    case 3: //时间
    {
        QSpinBox *spinBox = qobject_cast<QSpinBox *>(editor);
        model->setData(index, spinBox->value());
        break;
    }
    default:
        return QStyledItemDelegate::setModelData(editor, model, index);
        break;
    }
}

void PatrolEditDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    switch (index.column()) {
    case 1:
    case 2:
    case 3: //时间
    {
        editor->setGeometry(option.rect);
        if (index.column() == 3) {
            //QSpinBox *spinBox = qobject_cast<QSpinBox *>(editor);
            //spinBox->setFocus();
            //spinBox->clearFocus();
        }
        break;
    }
    default:
        QStyledItemDelegate::updateEditorGeometry(editor, option, index);
        break;
    }
}

void PatrolEditDelegate::onCommitData()
{
    emit commitData(qobject_cast<QWidget *>(sender()));
    emit activated(m_currentRow);
}

void PatrolEditDelegate::setChannel(int channel)
{
    m_channel = channel;
}

#include "comboboxbitrate.h"
#include <QtDebug>
#include "mylineedit.h"

ComboBoxBitRate::ComboBoxBitRate(QWidget *parent) :
    ComboBox(parent)
{
    setEditable(true);

    QLineEdit *edit = lineEdit();
    if (edit)
    {
        connect(edit, SIGNAL(textEdited(QString)), this, SLOT(onLineEditTextEdited(QString)));
        connect(edit, SIGNAL(editingFinished()), this, SLOT(onLineEditEditingFinished()));
    }
    connect(this, SIGNAL(activated(QString)), this, SLOT(onActivated(QString)));
}

void ComboBoxBitRate::setCurrentBitRate(int bitrate)
{
    int index = findData(bitrate);
    if (index < 0)
    {
        setEditText(QString::number(bitrate));
    }
    else
    {
        setCurrentIndex(index);
    }

    emit currentBitRateSet(bitrate);
}

int ComboBoxBitRate::currentBitRate() const
{
    int bitrate = currentText().toInt();
    return bitrate;
}

void ComboBoxBitRate::onLineEditTextEdited(const QString &text)
{
    int bitRate = text.toInt();

    emit currentBitRateSet(bitRate);
}

void ComboBoxBitRate::onLineEditEditingFinished()
{
    int bitrate = currentText().toInt();
    emit currentBitRateEditingFinished(bitrate);
}

void ComboBoxBitRate::onActivated(const QString &text)
{
    int bitRate = text.toInt();

    emit currentBitRateSet(bitRate);
}

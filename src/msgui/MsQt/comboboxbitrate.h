#ifndef COMBOBOXBITRATE_H
#define COMBOBOXBITRATE_H

#include "combobox.h"

class ComboBoxBitRate : public ComboBox
{
    Q_OBJECT
public:
    explicit ComboBoxBitRate(QWidget *parent = nullptr);

    void setCurrentBitRate(int bitrate);
    int currentBitRate() const;

signals:
    void currentBitRateSet(int bitrate);
    void currentBitRateEditingFinished(int bitrate);

private slots:
    void onLineEditTextEdited(const QString &text);
    void onLineEditEditingFinished();
    void onActivated(const QString &text);
};

#endif // COMBOBOXBITRATE_H

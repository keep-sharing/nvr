#ifndef CHANNELCOPYDIALOG_H
#define CHANNELCOPYDIALOG_H

#include "BaseShadowDialog.h"

namespace Ui {
class ChannelCopyDialog;
}

class ChannelCopyDialog : public BaseShadowDialog
{
    Q_OBJECT

public:
    explicit ChannelCopyDialog(QWidget *parent = nullptr);
    ~ChannelCopyDialog();

    void setTitle(const QString &title);
    void setCount(int count);

    void setCurrentChannel(int channel);
    QList<int> checkedList(bool containsCurrent = true);
    quint64 checkedFlags(bool containsCurrent = true) const;

private slots:
    void onLanguageChanged();
    void on_pushButton_ok_clicked();

    void on_pushButton_cancel_clicked();

private:
    Ui::ChannelCopyDialog *ui;
};

#endif // CHANNELCOPYDIALOG_H

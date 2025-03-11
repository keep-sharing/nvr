#ifndef IMAGEPAGEOSD_H
#define IMAGEPAGEOSD_H

#include "abstractimagepage.h"

extern "C"
{
#include "msg.h"
}

namespace Ui {
class ImagePageOsd;
}

class ImagePageOsd : public AbstractImagePage
{
    Q_OBJECT

public:
    explicit ImagePageOsd(QWidget *parent = nullptr);
    ~ImagePageOsd();

    void initializeData(int channel) override;
    void processMessage(MessageReceive *message) override;

private:
    void clearSettings();
    void setSettingsEnable(bool enable);

    void ON_RESPONSE_FLAG_OVF_GET_OSD(MessageReceive *message);
    void ON_RESPONSE_FLAG_OVF_SET_OSD(MessageReceive *message);

    void sendCopyData();
	bool isInputValid();
    void statusChange();

private slots:
    void onLanguageChanged();

    void on_comboBox_stream_activated(int index);
    void on_checkBox_showTitle_clicked(bool checked);
    void on_lineEdit_videoTitle_textEdited(const QString &text);
    void on_comboBox_titlePosition_activated(int index);
    void on_checkBox_showTimestamp_clicked(bool checked);
    void on_comboBox_datePosition_activated(int index);
    void on_comboBox_dateFormat_activated(int index);
    void on_comboBoxFontSize_activated(int index);

    void on_pushButton_copy_clicked();
    void on_pushButton_apply_clicked();
    void on_pushButton_back_clicked();

private:
    Ui::ImagePageOsd *ui;

    int m_currentChannel = 0;
    req_ovf_osd m_ovf_osd;

    QList<int> m_copyChannelList;
};

#endif // IMAGEPAGEOSD_H

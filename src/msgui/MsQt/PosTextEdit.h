#ifndef POSTEXTEDIT_H
#define POSTEXTEDIT_H

#include <QPlainTextEdit>
#include "PosData.h"

//TODO: LiuHuanyu 2021-08-13, pos测试没问题后可能删除此文件

class PosTextEdit : public QPlainTextEdit {
    Q_OBJECT

public:
    explicit PosTextEdit(QWidget *parent = nullptr);

    void appendText(const PosData &data);
    void setPaused(bool pause);

    //根据POS信息和视频窗口调整
    void setPosGeometry(const PosData &data, const QRect &videoRect);
    //根据视频窗口调整，POS信息用上一次的
    void resetPosGeometry(const QRect &videoRect);

protected:
    void paintEvent(QPaintEvent *e) override;

private slots:
    void scrollToBottom();
    void onTimeout();

private:
    QTextCursor m_textCursor;

    //100毫秒为单位，50*100=5秒
    QTimer *m_timeout = nullptr;
    int m_timeoutValue = 0;
    int m_timeoutInterval = 50;

    //缓存上一次pos原生区域
    QRect m_posArea;
};

#endif // POSTEXTEDIT_H

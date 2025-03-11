#ifndef LIVEVIEWTARGETITEM_H
#define LIVEVIEWTARGETITEM_H

#include <QWidget>
#include <QDateTime>
#include "TargetInfoManager.h"

namespace Ui {
class LiveViewTargetItem;
}

class LiveViewTargetItem : public QWidget
{
    Q_OBJECT

public:
    explicit LiveViewTargetItem(QWidget *parent = nullptr);
    ~LiveViewTargetItem();

    QRect globalGeometry() const;

    void setIndex(int index);
    void setTempHide(bool hide, bool hideBorder = false);

    int channel() const;
    QString plate() const;
    QDateTime anprTime() const;

    void updateItemInfo();
    void clearItemInfo();
    bool hasInfo() const;
    int infoType() const;

    void setNetworkFocus(bool focus);

protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

signals:
    void itemClicked(int index);
    void itemContextMenuRequested(int index, const QPoint &pos);

private slots:
    void onBorderStateChanged(bool visible, const QColor &color);

private:
    Ui::LiveViewTargetItem *ui;

    int m_index = -1;
    int m_infoType = 0;

    int m_channel = -1;
    QString m_time;

    QString m_plate;
    ToolButtonFace *m_plateIcon = nullptr;

    QColor m_borderColor = QColor("#FFFFFF");
    bool m_isTempHide = false;
    bool m_isHideBorder = false;

    bool m_borderVisible = false;

    //
    bool m_isNetworkFocus = false;
};

#endif // LIVEVIEWTARGETITEM_H

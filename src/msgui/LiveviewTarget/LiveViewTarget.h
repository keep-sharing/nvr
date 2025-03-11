#ifndef LIVEVIEWTARGET_H
#define LIVEVIEWTARGET_H

#include <QWidget>
#include "LiveViewTargetItem.h"
#include "LiveViewTargetPlay.h"
#include "AnprItemMenu.h"

namespace Ui {
class LiveViewTarget;
}

class LiveViewTarget : public QWidget
{
    Q_OBJECT

public:
    explicit LiveViewTarget(QWidget *parent = nullptr);
    ~LiveViewTarget();

    static LiveViewTarget *instance();

    void setTargetEnable(bool enable, int regson);
    bool isTargetEnable() const;

    void clearTargetInfo();

    void showNoResource(const QMap<int, bool> &map);

    bool isPlaying();
    void setNetworkFocus(bool focus);
    bool isNetworkFocus();
    void networkFocusNext();
    void networkFocusPrevious();
    void hideNetworkFocus();
    void showNetworkFocus();
    void networkPlay();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void paintEvent(QPaintEvent *) override;

private:
    void addAnprlist(const QString &plate, const QString &type);
    void removeAnprList(const QString &plate);
    void updateItemType();

    void updateTargetrInfo();

private slots:
    void onLanguageChanged();

    void onItemClicked(int index);
    void onItemContextMenuRequested(int index, const QPoint &pos);

    void onTargetInfoChanged();
    void onTargetInfoCleared();

    //menu
    void onAddToBlacklist(const QString &plate);
    void onAddToWhitelist(const QString &plate);
    void onRemoveFromList(const QString &plate);

    void on_toolButtonSettings_clicked();

private:
    static LiveViewTarget *s_liveViewTarget;

    Ui::LiveViewTarget *ui;

    bool m_isEnable = false;

    QList<LiveViewTargetItem *> m_itemList;

    LiveViewTargetPlay *m_anprPlay = nullptr;
    bool m_isPlaying = false;

    AnprItemMenu *m_itemMenu = nullptr;

    //
    bool m_isNetwokFocus = false;
    int m_networkFocusIndex = -1;

    QRect m_videoRect;
};

#endif // LIVEVIEWTARGET_H

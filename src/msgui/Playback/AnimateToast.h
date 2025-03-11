#ifndef ANIMATETOAST_H
#define ANIMATETOAST_H

#include <QDialog>
#include <QPropertyAnimation>
#include <QTimer>
#include "AnimateToastItem.h"

namespace Ui {
class AnimateToast;
}

class AnimateToast : public QDialog
{
    Q_OBJECT

public:
    explicit AnimateToast(QWidget *parent = nullptr);
    ~AnimateToast();

    static AnimateToast *s_animateToast;

    void setDestWidget(QWidget *widget);

    void showToast(const QString &str);
    void showToast(const QString &str, int msec);
    void hideToast();
    void hideToastLater(int msec);
    void setToastText(const QString &str);
    void startAnimationLater(int msec, QWidget *dest = nullptr);

    void addItem(const QString &strChannel, const QString &strResult);
    void addItemMap(const QMap<int, QString> &map);

    void setCount(int count);
    void successAdded();
    void failAdded();
    void indexAdded();

    bool isEnd();

    int successCount();

protected:
    void closeEvent(QCloseEvent *) override;

private:
    void clearItem();

private slots:
    void onTimeout();

private:
    Ui::AnimateToast *ui;

    QPropertyAnimation *m_animation = nullptr;

    QList<AnimateToastItem *> m_itemList;
    QTimer *m_timer;
    QTimer *m_timerAutoHide;

    int m_totalCount = 0;
    int m_index = 0;
    int m_successCount = 0;
    int m_failedCount = 0;

    QWidget *m_destWidget = nullptr;
};

#endif // ANIMATETOAST_H

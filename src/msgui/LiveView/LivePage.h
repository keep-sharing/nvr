#ifndef LIVEPAGE_H
#define LIVEPAGE_H

#include <QWidget>
#include <QTimer>

namespace Ui {
class LivePage;
}

class LivePage : public QWidget
{
    Q_OBJECT

public:
    enum Mode
    {
        ModeAlwaysShow,
        ModeAlwaysHide,
        ModeAuto
    };

    explicit LivePage(QWidget *parent = 0);
    ~LivePage();

    static LivePage *instance();
    static void setMainPage(LivePage *page);
    static LivePage *mainPage();
    static void setSubPage(LivePage *page);
    static LivePage *subPage();

    static void initializeData();
    static void setMode(LivePage::Mode mode);
    static LivePage::Mode mode();

    void showOrHide(const QPoint &pos);
    void setPage(int page, int count);
    void showPage(int msec);

protected:
    void showEvent(QShowEvent *) override;
    void hideEvent(QHideEvent *) override;

private:
    void setPageMode(LivePage::Mode mode);

signals:
    void nextPage();
    void previousPage();

public slots:
    void adjustPos(const QRect &parentRect);

private slots:
    void onTimeout();

    void on_toolButton_previousPage_clicked();
    void on_toolButton_nextPage_clicked();

private:
    Ui::LivePage *ui;
    static LivePage *s_livePage;
    static LivePage *s_mainPage;
    static LivePage *s_subPage;

    static Mode s_mode;

    int m_pageCount = 0;
    int m_currentPage = 0;

    int m_height = 70;
    QRect m_enterRect;

    QTimer *m_timer;
    //避免频繁点击
    QTimer *m_limitTimer = nullptr;
};

#endif // LIVEPAGE_H

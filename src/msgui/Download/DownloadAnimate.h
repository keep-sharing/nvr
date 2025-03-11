#ifndef DOWNLOADANIMATE_H
#define DOWNLOADANIMATE_H

#include <QWidget>

class QPropertyAnimation;

class DownloadAnimate : public QWidget
{
    Q_OBJECT
public:
    explicit DownloadAnimate(QWidget *parent = 0);

    static DownloadAnimate *instance();

    void showAnimate(const QRect &rc, const QPixmap &pixmap);
    void startAnimate(const QRect &rc, const QPixmap &pixmap);

protected:
    void paintEvent(QPaintEvent *) override;

signals:

public slots:

private:
    static DownloadAnimate *self;

    QPropertyAnimation *m_animation = nullptr;
    QPixmap m_pixmap;
};

#endif // DOWNLOADANIMATE_H

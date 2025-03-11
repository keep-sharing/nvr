#ifndef CONTENTLIVEVIEW_H
#define CONTENTLIVEVIEW_H

#include <QWidget>

class ContentLiveView : public QWidget
{
    Q_OBJECT
public:
    explicit ContentLiveView(QWidget *parent = 0);
    ~ContentLiveView();

    static ContentLiveView *instance();

    void setGeometry(const QRect &rc);

signals:

public slots:

private:
    static ContentLiveView *self;
};

#endif // CONTENTLIVEVIEW_H

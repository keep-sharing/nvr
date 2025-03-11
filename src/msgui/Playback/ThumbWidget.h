#ifndef THUMBWIDGET_H
#define THUMBWIDGET_H

#include <QWidget>

class ThumbWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ThumbWidget(QWidget *parent = nullptr);

    void setImage(const QImage &image);

protected:
    void paintEvent(QPaintEvent *) override;

signals:

public slots:

private:
    QImage m_image;
};

#endif // THUMBWIDGET_H

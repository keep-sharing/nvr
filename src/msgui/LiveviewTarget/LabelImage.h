#ifndef LABELIMAGE_H
#define LABELIMAGE_H

#include <QLabel>

class LabelImage : public QLabel
{
    Q_OBJECT
public:
    enum ResizeMode
    {

    };

    explicit LabelImage(QWidget *parent = nullptr);

    void setImage(const QImage &image);
    void clearImage();
    QImage image() const;

    void setSmoothPixmapTransform(bool enable);

    void setRatio(const qreal &ratio);

protected:
    void paintEvent(QPaintEvent *) override;

signals:

public slots:

private:
    QImage m_image;

    bool m_isSmoothPixmapTransform = false;
    qreal m_ratio = 16.0 / 9;
};

#endif // LABELIMAGE_H

#ifndef TARGETIMAGEWIDGET_H
#define TARGETIMAGEWIDGET_H

#include <QWidget>

class TargetImageWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TargetImageWidget(QWidget *parent = nullptr);

    void setIndex(int index);

protected:
    void paintEvent(QPaintEvent *) override;

signals:

public slots:

private:
    int m_index = -1;
};

#endif // TARGETIMAGEWIDGET_H

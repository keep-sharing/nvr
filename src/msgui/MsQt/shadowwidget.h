#ifndef SHADOWWIDGET_H
#define SHADOWWIDGET_H

#include <QWidget>

class ShadowWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ShadowWidget(QWidget *parent = 0);

protected:
    void paintEvent(QPaintEvent *event) override;

signals:

public slots:

private:
    int m_shadowWidth = 11;
    QColor m_backgroundColor;
};

#endif // SHADOWWIDGET_H

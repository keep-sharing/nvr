#ifndef QRCODEWIDGET_H
#define QRCODEWIDGET_H

#include <QWidget>
#include "qrencode/qrencode.h"

class QRCodeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit QRCodeWidget(QWidget *parent = nullptr);
    ~QRCodeWidget();

    void setText(const QString &text);

protected:
    void paintEvent(QPaintEvent *) override;

signals:

public slots:

private:
    QString m_text;
    QRcode *m_code = nullptr;
};

#endif // QRCODEWIDGET_H

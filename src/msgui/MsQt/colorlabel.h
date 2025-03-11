#ifndef COLORLABEL_H
#define COLORLABEL_H

#include <QLabel>

class ColorLabel : public QLabel
{
    Q_OBJECT
public:
    explicit ColorLabel(QWidget *parent = nullptr);

    void setColor(const QColor &color);
    void setColor(const QString &color);

signals:

public slots:
};

#endif // COLORLABEL_H

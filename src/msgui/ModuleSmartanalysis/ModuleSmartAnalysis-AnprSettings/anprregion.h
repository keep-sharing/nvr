#ifndef ANPRREGION_H
#define ANPRREGION_H

#include "maskwidget.h"

class AnprRegion : public MaskWidget
{
    Q_OBJECT
public:
    explicit AnprRegion(QWidget *parent = nullptr);
    explicit AnprRegion(int index, QWidget *parent = nullptr);

    void setIndex(int index);
    int index() const;

    void setName(const QString &name);
    QString name();

    QRect realRect() const;
    void setRealRect(const QRect &rc);

protected:
    void paintEvent(QPaintEvent *event) override;

signals:

public slots:

private:
    int m_index = -1;
    QString m_name;
};

#endif // ANPRREGION_H

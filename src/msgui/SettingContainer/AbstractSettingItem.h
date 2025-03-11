#ifndef ABSTRACTSETTINGITEM_H
#define ABSTRACTSETTINGITEM_H

#include <QWidget>

class AbstractSettingItem : public QWidget
{
    Q_OBJECT
public:
    explicit AbstractSettingItem(QWidget *parent = nullptr);

signals:

public slots:
};

#endif // ABSTRACTSETTINGITEM_H

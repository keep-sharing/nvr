#ifndef ABSTRACTCAMERAPAGE_H
#define ABSTRACTCAMERAPAGE_H

#include "MsWidget.h"

class AbstractCameraPage : public MsWidget
{
    Q_OBJECT
public:
    explicit AbstractCameraPage(QWidget *parent = nullptr);

    void back();

    virtual void initializeData() = 0;
    virtual void dealMessage(MessageReceive *message);

    virtual bool isCloseable();
    virtual bool isChangeable();
    virtual bool canAutoLogout();

public slots:
    void showWait();
    void closeWait();

signals:
    void sig_back();
};

#endif // ABSTRACTCAMERAPAGE_H

#ifndef BASEWIDGET_H
#define BASEWIDGET_H

#include "networkcommond.h"
#include <QMutex>
#include "MsWidget.h"

class BaseWidget : public MsWidget {
    Q_OBJECT

public:
    explicit BaseWidget(QWidget *parent = nullptr);
    ~BaseWidget() override;

    static BaseWidget *currentWidget();
    static QList<BaseWidget *> visibleList();

    virtual NetworkResult dealNetworkCommond(const QString &commond);

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

    virtual void focusPreviousChild();
    virtual void focusNextChild();

    virtual void escapePressed();
    virtual void returnPressed();

    virtual bool isAddToVisibleList();

    //网络键盘
    virtual NetworkResult dealRockerNvr(const RockerDirection &direction);
    virtual NetworkResult dealRockerPtz(const RockerDirection &direction, int hRate, int vRate);
    virtual NetworkResult deal_Dial_Insid_Add();
    virtual NetworkResult deal_Dial_Insid_Sub();

    //
    void sendMessage(int type, const void *data, int size);
    void sendMessageOnly(int type, const void *data, int size);

    //
    void loadStylesheet(const QString &filePath);

signals:

public slots:

protected:
    static QMutex s_mutex;

private:
    static QList<BaseWidget *> s_visibleList;
};

#endif // BASEWIDGET_H

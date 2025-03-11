#ifndef BASESETTING_H
#define BASESETTING_H

#include "commonvideo.h"
#include "BaseWidget.h"
#include "settinglistitem.h"

class MessageReceive;

class BaseSetting : public BaseWidget {
    Q_OBJECT
    Q_ENUMS(SettingType)

public:
    enum SettingType {
        TypeRetrueve,
        TypeLayout,
        TypeCamera,
        TypeStorage,
        TypeEvent,
        TypeSettings,
        TypeStatus
    };

    explicit BaseSetting(SettingType type, QWidget *parent = 0);
    virtual ~BaseSetting();

    SettingType settingType() const;

    virtual void initializeSetting();
    virtual void closeSetting();

    virtual void closeCurrentPage();

    virtual bool isCloseable();
    virtual bool isChangeable();
    virtual bool canAutoLogout();

    virtual void dealMessage(MessageReceive *message);
    virtual QList<SettingItemInfo> itemList() = 0;
    virtual void setCurrentItem(int item_id) = 0;
    virtual int currentItem() = 0;

protected:
    void closeEvent(QCloseEvent *) override;

signals:
    void sig_close();
    void updateVideoGeometry();

public slots:

private:
    SettingType m_type;
};

#endif // BASESETTING_H

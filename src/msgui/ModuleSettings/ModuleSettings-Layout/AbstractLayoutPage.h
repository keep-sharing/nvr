#ifndef ABSTRACTLAYOUTPAGE_H
#define ABSTRACTLAYOUTPAGE_H

#include <QWidget>

class MessageReceive;
class MsWaitting;

class AbstractLayoutPage : public QWidget {
    Q_OBJECT
public:
    explicit AbstractLayoutPage(QWidget *parent = nullptr);

    virtual void initializeData() = 0;
    virtual void dealMessage(MessageReceive *message) = 0;

signals:
    void sig_back();

signals:

public slots:

protected:
    MsWaitting *m_waitting = nullptr;
};

#endif // ABSTRACTLAYOUTPAGE_H

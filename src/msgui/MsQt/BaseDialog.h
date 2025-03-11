#ifndef BASEDIALOG_H
#define BASEDIALOG_H

#include "BaseWidget.h"

#include <QEventLoop>

class BaseDialog : public BaseWidget {
    Q_OBJECT
public:
    explicit BaseDialog(QWidget *parent = nullptr);

    enum DialogCode {
        Rejected,
        Accepted
    };

protected:
    void showEvent(QShowEvent *event) override;

    virtual bool isMoveToCenter();

signals:
    void closed();
    void finished(int result);
    void accepted();
    void rejected();

public slots:
    int exec();
    bool close();
    virtual void done(int r);
    virtual void accept();
    virtual void reject();

private:
    QEventLoop m_dialogEventLoop;
};

#endif // BASEDIALOG_H

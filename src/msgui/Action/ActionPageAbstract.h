#ifndef ACTIONPAGEABSTRACT_H
#define ACTIONPAGEABSTRACT_H

#include <QWidget>

class ActionAbstract;
class MyButtonGroup;
class MessageReceive;

struct schedule_day;

class ActionPageAbstract : public QWidget {
    Q_OBJECT

public:
    explicit ActionPageAbstract(QWidget *parent);

    bool hasCache() const;
    void setCached();
    void clearCache();
    virtual void dealMessage(MessageReceive *message);

protected:
    void showEvent(QShowEvent *) override;

    virtual int loadData() = 0;
    virtual int saveData() = 0;

    static bool hasSchedule(schedule_day *schedule_day_array);

signals:

public slots:
    virtual void onLanguageChanged();
    void onCancelClicked();

protected:
    ActionAbstract *m_action = nullptr;

    friend class ActionAbstract;

private:
    bool m_cache = false;
};

#endif // ACTIONPAGEABSTRACT_H

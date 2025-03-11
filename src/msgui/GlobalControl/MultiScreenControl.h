#ifndef MULTISCREENCONTROL_H
#define MULTISCREENCONTROL_H

#include <QObject>
#include <QRect>

extern "C" {
#include "msdb.h"
}

#define gMultiScreenControl MultiScreenControl::instance()

class MultiScreenControl : public QObject {
    Q_OBJECT
public:
    explicit MultiScreenControl(QObject *parent = nullptr);
    ~MultiScreenControl();

    static MultiScreenControl &instance();

    int currentScreen() const;
    bool isSubEnable() const;

    int multiScreenSupport();

    QRect mainScreenGeometry() const;
    QRect subScreenGeometry() const;

signals:

public slots:

private:
    struct display m_db_display;
};

#endif // MULTISCREENCONTROL_H

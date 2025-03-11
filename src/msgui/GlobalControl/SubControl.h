#ifndef SUBCONTROL_H
#define SUBCONTROL_H

#include "MsObject.h"
#include <QLocalServer>
#include <QLocalSocket>
#include <QProcess>

extern "C" {
#include "msdb.h"
#include "vapi.h"
}

class QTimer;

#define gSubControl SubControl::instance()

class SubControl : public MsObject {
    Q_OBJECT

public:
    explicit SubControl(QObject *parent = nullptr);
    ~SubControl();

    static SubControl *instance();

    static void initializeMultiScreen();
    static void setMultiSupported(bool support);
    static bool isMultiSupported();

    static void setMainScreenGeometry(const QRect &rc);
    static void setSubScreenGeometry(const QRect &rc);
    static QRect mainScreenGeometry();
    static QRect subScreenGeometry();
    static QRect currentScreenGeometry();

    static QString screenString(int screen);

    void setSubEnable(bool enable);
    static bool isSubEnable();

    void setSubControl(bool enable);
    bool isSubControl();

    SCREEN_E startScreen();
    SCREEN_E currentScreen();
    SCREEN_E mainLiveViewScreen();
    SCREEN_E subLiveViewScreen();

    QRect logicalMainScreenGeometry();
    QRect logicalSubScreenGeometry();
    QRect physicalMainScreenGeometry();
    QRect physicalSubScreenGeometry();

    QRect mapToNvrRect(const QRect qtRect) const;

    void switchScreen();
    void switchFrameBuffer(const QString &displaySpec, bool clearFB = true);

private:
    void readDisplayInfo();

signals:
    void screenSwitched();

public slots:
    void initializeLater();

private slots:
    void onMiddleButtonDoubleClicked();
    void onSwitchScreen();

private:
    static SubControl *s_subControl;
    static bool s_multiSupported;
    static bool s_isSubEnable;
    static QRect s_mainScreenGeometry;
    static QRect s_subScreenGeometry;

    struct display m_db_display;

    bool m_isSubControl = false;
    bool m_isSubEnable = false;
    SCREEN_E m_startScreen;

    QTimer *m_switchTimer = nullptr;
};

#endif // SUBCONTROL_H

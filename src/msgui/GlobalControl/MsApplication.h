#ifndef MSAPPLICATION_H
#define MSAPPLICATION_H

#include <QApplication>
#include <QElapsedTimer>
#include <QPointer>
#include <QRect>
#include <QTimer>

class MainWindow;

#define qMsApp (static_cast<MsApplication *>(QCoreApplication::instance()))

class MsApplication : public QApplication {
    Q_OBJECT
public:
    explicit MsApplication(int &argc, char **argv);
    ~MsApplication() override;

    void setMainWindow(MainWindow *window);
    void setInitializeFinished(bool finished);
    bool isInitializeFinished() const;

    void setAboutToReboot(bool value);

    bool notify(QObject *receiver, QEvent *e) override;

private:
    void mouseActive();

signals:
    void middleButtonDoubleClicked();

private slots:
    void onTimerHideCursor();

private:
    MainWindow *m_mainWindow = nullptr;
    bool m_initializeFinished = false;
    QTimer *m_timerHideCursor = nullptr;

    bool m_isLeftButtonPressed = false;
    bool m_isRightButtonPressed = false;

    bool m_isAboutToReboot = false;
};

#endif // MSAPPLICATION_H

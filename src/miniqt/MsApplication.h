#ifndef MSAPPLICATION_H
#define MSAPPLICATION_H

#include <QApplication>
#include <QElapsedTimer>
#include <QPointer>
#include <QTimer>

class MainWindow;

#define msApp (static_cast<MsApplication *>(QCoreApplication::instance()))

class MsApplication : public QApplication {
    Q_OBJECT

  public:
    explicit MsApplication(int &argc, char **argv);
    ~MsApplication() override;

    bool notify(QObject *receiver, QEvent *e) override;
    bool qwsEventFilter(QWSEvent *e) override;

  private slots:
    void onTimer();

  private:
    QTimer *m_timer      = nullptr;
    bool    m_needUpdate = false;
};

#endif // MSAPPLICATION_H

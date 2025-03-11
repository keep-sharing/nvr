#pragma once

#include <QAtomicInt>
#include <QDialog>
#include <QTimer>

class MsWaitting : public QDialog {
    Q_OBJECT

public:
    explicit MsWaitting(QWidget *parent);
    ~MsWaitting() override;

    static MsWaitting *instance();

    static int globalRef();
    static int globalDeref();
    static void showGlobalWait(QWidget *parent = nullptr);
    static void execGlobalWait(QWidget *parent = nullptr);
    static void closeGlobalWait();

    static bool hasWaitting();

    int userResult();
    void setUserResult(int result);

    void setCustomPos(bool enable);
    void moveToCenter(const QRect &rc);

    void showWait();
    int execWait();
    void exitWait(int value);
    void showWait(const QString &message);
    int execWait(const QString &message);
    void showWait(QWidget *parent, const QString &message);
    int execWait(QWidget *parent, const QString &message);
    void closeWait();
    void closeWait(const QString &message);

    int ref();
    int deref();

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void paintEvent(QPaintEvent *) override;

    void mousePressEvent(QMouseEvent *event) override;

private slots:
    void onTimeout();

private:
    static MsWaitting *s_msWaitting;

    static QHash<MsWaitting *, bool> s_waittingMap;

    QWidget *m_parent;
    QTimer *m_timer;

    QColor m_brushColor;
    int m_angle;

    int m_userResult = 0;

    bool m_isCustomPos = false;

    //
    int m_clickCount = 0;

    QAtomicInt m_ref = 0;
};

class MsWaittingContainer {
public:
    inline explicit MsWaittingContainer(QWidget *parent = nullptr)
    {
        Q_UNUSED(parent)
        //MsWaitting::showGlobalWait(parent);
        MsWaitting::globalRef();
    }
    inline explicit MsWaittingContainer(MsWaitting *waitting)
    {
        m_waitting = waitting;
        if (m_waitting) {
            //m_waitting->//showWait();
        }
    }
    inline ~MsWaittingContainer()
    {
        if (m_waitting) {
            //m_waitting->//closeWait();
        } else {
            if (!MsWaitting::globalDeref()) {
                //MsWaitting::closeGlobalWait();
            }
        }
    }

private:
    MsWaitting *m_waitting = nullptr;
};

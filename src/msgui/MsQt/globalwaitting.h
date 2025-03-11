#ifndef GLOBALWAITTING_H
#define GLOBALWAITTING_H

#include "MsWaitting.h"

class GlobalWaitting : public MsWaitting
{
    Q_OBJECT
public:
    explicit GlobalWaitting(QWidget *parent = nullptr);

    static GlobalWaitting *instance();

    static void showWait();
    static void showWait(const QRect &rc);
    static void showWait(QWidget *widget);
    static int execWait(const QRect &rc = QRect());
    static int execWait(QWidget *widget = nullptr);
    static void closeWait();

    void moveToScreenCenter();

private:

signals:

private:
    static GlobalWaitting *s_gWaitting;
};

#endif // GLOBALWAITTING_H

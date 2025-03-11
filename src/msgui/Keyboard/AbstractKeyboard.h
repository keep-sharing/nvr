#ifndef ABSTRACTKEYBOARD_H
#define ABSTRACTKEYBOARD_H

#include <QWidget>

class AbstractKeyboard : public QWidget
{
    Q_OBJECT
public:
    explicit AbstractKeyboard(QWidget *parent = nullptr);

    virtual QString buttonText(const QString &name) const;

signals:
    void sigClose();
};

#endif // ABSTRACTKEYBOARD_H

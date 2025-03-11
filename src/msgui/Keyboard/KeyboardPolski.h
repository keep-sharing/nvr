#ifndef KEYBOARDPOLSKI_H
#define KEYBOARDPOLSKI_H

#include "AbstractKeyboard.h"
#include "KeyboardData.h"
#include <QMap>
#include <QWidget>

class KeyButton;

namespace Ui {
class KeyboardPolski;
}

class KeyboardPolski : public AbstractKeyboard {
    Q_OBJECT

public:
    explicit KeyboardPolski(QWidget *parent = nullptr);
    ~KeyboardPolski();

    QString buttonText(const QString &name) const override;

protected:
    void showEvent(QShowEvent *) override;
    void hideEvent(QHideEvent *) override;

private:
    void dealControls(const KeyboardData::Controls &cs);
    void dealKeyEvent(const Qt::Key &k);
    void dealKeyValue(const ushort &s);

private slots:
    void onKeyButtonClicked(const QString &name);

private:
    Ui::KeyboardPolski *ui = nullptr;

    QMap<QString, KeyButton *> m_keyMap;
};

#endif // KEYBOARDPOLSKI_H

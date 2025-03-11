#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <QWidget>
#include <QElapsedTimer>

class AbstractKeyboard;

namespace Ui {
class Keyboard;
}

class Keyboard : public QWidget
{
    Q_OBJECT

public:
    explicit Keyboard(QWidget *parent = nullptr);
    ~Keyboard();

protected:
    bool eventFilter(QObject *, QEvent *) override;
    void paintEvent(QPaintEvent *) override;

private slots:
    void onLanguageChanged();
    void onKeyboardClose();

private:
    Ui::Keyboard *ui;

    AbstractKeyboard *m_keyboard = nullptr;

    QPoint m_dragPosition;
    QElapsedTimer m_dragThreshold;
};

#endif // KEYBOARD_H

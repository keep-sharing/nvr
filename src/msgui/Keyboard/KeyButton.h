#ifndef KEYBUTTON_H
#define KEYBUTTON_H

#include <QToolButton>

class AbstractKeyboard;

class KeyButton : public QToolButton
{
    Q_OBJECT
public:
    explicit KeyButton(QWidget *parent = nullptr);

    QString initialize(AbstractKeyboard *keyboard);

protected:
    bool qwsEvent(QWSEvent *event) override;
    void mousePressEvent(QMouseEvent * event) override;
    void mouseReleaseEvent(QMouseEvent * event) override;
    void enterEvent(QEvent *) override;
    void leaveEvent(QEvent *) override;

    void showEvent(QShowEvent *) override;
    void hideEvent(QHideEvent *event) override;

    void paintEvent(QPaintEvent *) override;

signals:
    void keyClicked(const QString &name);

private slots:
    void onClicked();

private:
    bool m_qwsPressed = false;
    int m_pressCount = 0;

    bool m_isPressed = false;
    bool m_isHover = false;
    QString m_name;

    AbstractKeyboard *m_keyboard = nullptr;
};

#endif // KEYBUTTON_H

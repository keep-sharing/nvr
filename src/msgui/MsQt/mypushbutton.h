#ifndef MYPUSHBUTTON_H
#define MYPUSHBUTTON_H

#include <QPushButton>

class MyPushButton : public QPushButton
{
    Q_OBJECT
public:
    explicit MyPushButton(QWidget *parent = nullptr);
    explicit MyPushButton(const QString &text, QWidget *parent = nullptr);

    void setTranslatableText(const QString &key, const QString &defaultValue);
    void retranslate();

    void clearUnderMouse();
    void clearHover();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
	
	void keyPressEvent(QKeyEvent *event) override;

private:
    QString m_textKey;
    QString m_defaultValue;
};

#endif // MYPUSHBUTTON_H

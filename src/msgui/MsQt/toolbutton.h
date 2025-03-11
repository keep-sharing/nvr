#ifndef TOOLBUTTON_H
#define TOOLBUTTON_H

#include <QToolButton>

class ToolButton : public QToolButton
{
    Q_OBJECT
public:
    explicit ToolButton(QWidget *parent = 0);

    void setText(const QString &text);
    QString text() const;

    void setPixmap(const QPixmap &pixmap);
    void setHoverPixmap(const QPixmap &pixmap);
    void setPressedPixmap(const QPixmap &pixmap);

    void setTextColor(const QColor &color);
    void setTextHoverColor(const QColor &color);
    void setTextPressedColor(const QColor &color);

    void setTextPixel(int pixel);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void focusInEvent(QFocusEvent *e) override;
    void focusOutEvent(QFocusEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;

signals:

public slots:

private:
    QString m_text;

    QPixmap m_pixmap;
    QPixmap m_hoverPixmap;
    QPixmap m_pressedPixmap;

    QColor m_textColor;
    QColor m_textHoverColor;
    QColor m_textPressedColor;

    int m_textPixel = -1;

    bool m_isPressed = false;
    bool m_isUnderMouse = false;
};

#endif // TOOLBUTTON_H

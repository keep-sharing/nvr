#ifndef BASESLIDER_H
#define BASESLIDER_H

#include <QSlider>

class BaseSlider : public QSlider
{
    Q_OBJECT
public:
    explicit BaseSlider(QWidget *parent = nullptr);

    quint32 value() const;
    void setRange(quint32 min, quint32 max);
    QPair<quint32, quint32> range();
    void setMaximum(int value);
    void setMinimum(int value);

    quint32 maximunValue() const;
    quint32 minimumValue() const;

    void setTextColor(const QColor &color);

    void setShowValue(bool show);

public slots:
    void setValue(int value);
    //不会触发signals
    void editValue(int value);

protected:
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

    virtual int rightMarginWithoutText() const;
    virtual int rightMarginWithText() const;

    virtual QString tipText();
    virtual QString valueText();

    virtual QColor grooveNormalColor();
    virtual QColor grooveDisableColor();
    virtual QColor grooveValueNormalColor();
    virtual QColor grooveValueDisableColor();
    virtual QColor tipNormalColor();
    virtual QColor tipDisableColor();
    virtual QColor valueNormalColor();
    virtual QColor valueDisableColor();
    virtual void drawValue(QPainter &painter);
    virtual void drawTip(QPainter &painter);

    quint32 valueUnderMouse(const QPoint &pos);

private:
    int rightMargin() const;

signals:
    /**
     * @brief sliderMoved
     * 界面拖动时触发
     * @param value
     */
    void sliderMoved(int value);

    /**
     * @brief valueChanged
     * 调用setValue或者界面点击触发
     * 拖动时不会触发，鼠标松开后触发
     * @param value
     */
    void valueChanged(int value);

    /**
     * @brief valueEdited
     * 调用setValue不会触发
     * 界面点击触发
     * 拖动时不会触发，鼠标松开后触发
     * @param value
     */
    void valueEdited(int value);

public slots:

protected:
    bool m_drawTip = false;
    bool m_pressed = false;
    QPoint m_mouseMovePos;

    bool m_isDragging = false;
    quint32 m_dragValue = 0;

    bool m_isShowValue = true;
    quint32 m_value = 0;
    quint32 m_minValue = 0;
    quint32 m_maxValue = 100;

    //m_handleWidth / 2
    int m_marginLeft = 8;

    int m_grooveHeight = 5;
    int m_handleWidth = 16;
    int m_handleHeight = 16;

    QRect m_grooveRect;
    QRect m_valueGrooveRect;

    QColor m_textColor = QColor("#4A4A4A");
    QColor m_disabledColor = QColor("#BEBEBE");
};

#endif // BASESLIDER_H

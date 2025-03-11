#ifndef MYTOOLBUTTON_H
#define MYTOOLBUTTON_H

#include <QMap>
#include <QToolButton>
#include <QVariant>

class MyToolButton : public QToolButton {
    Q_OBJECT
public:
    explicit MyToolButton(QWidget *parent = nullptr);

    void clearUnderMouse();

    void setData(const QVariant &value, int role);
    QVariant data(int role) const;

protected:
    void focusOutEvent(QFocusEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *) override;

signals:
    void mouseEnter();

public slots:

private:
    QMap<int, QVariant> m_dataMap;
};

#endif // MYTOOLBUTTON_H

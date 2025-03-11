#pragma once
#include <QToolButton>
#include <QWSEvent>

class Key : public QToolButton {
    Q_OBJECT

public:
    Key(QWidget *parent);

protected:
    bool qwsEvent(QWSEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private:
};

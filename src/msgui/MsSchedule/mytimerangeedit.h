#ifndef MYTIMERANGEEDIT_H
#define MYTIMERANGEEDIT_H

#include <QAbstractSpinBox>

class QToolButton;

class MyTimeRangeEdit : public QAbstractSpinBox
{
    Q_OBJECT
public:
    explicit MyTimeRangeEdit(QWidget *parent = nullptr);

    void setTime(int begin, int end);

    void stepBy(int steps) override;

public slots:
    void clearTime();

protected:
    void showEvent(QShowEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

    void wheelEvent(QWheelEvent *event) override;

    void focusInEvent(QFocusEvent *event);
    void focusOutEvent(QFocusEvent *event) override;

    virtual StepEnabled stepEnabled() const;

private:
    void adjustToolPosition();

private slots:
    void onToolButtonClear();

signals:
    void timeCleared(int begin, int end);
    void timeEditingFinished(int begin, int end);

private:
    QToolButton *m_toolClear = nullptr;

    int m_previousBegin = 0;
    int m_previousEnd = 0;

    int m_h1 = 0;
    int m_m1 = 0;
    int m_h2 = 0;
    int m_m2 = 0;
};

#endif // MYTIMERANGEEDIT_H

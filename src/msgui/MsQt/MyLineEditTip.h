#ifndef MYLINEEDITTIP_H
#define MYLINEEDITTIP_H

#include <QWidget>

class QToolButton;

class MyLineEditTip : public QWidget {
    Q_OBJECT

public:
    explicit MyLineEditTip(QWidget *parent = nullptr);
    ~MyLineEditTip() override;

    const QString &text() const;
    void setText(const QString &newText);
    void setTextSize(int size);

signals:
    void onEnableFlase();

protected:
    void resizeEvent(QResizeEvent *) override;
    void showEvent(QShowEvent *) override;
    void paintEvent(QPaintEvent *) override;

private:
    QToolButton *m_toolButton = nullptr;
    QColor m_backgroundColor = QColor(255, 255, 255);
    QColor m_borderColor = QColor(231, 94, 93);
    QString m_text;
    int m_fontSize = 0;

};

#endif // MYLINEEDITTIP_H

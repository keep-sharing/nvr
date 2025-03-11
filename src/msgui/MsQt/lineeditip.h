#ifndef LINEEDITIP_H
#define LINEEDITIP_H

#include <QLineEdit>

class LineEditIP : public QLineEdit
{
    Q_OBJECT

public:
    LineEditIP(QWidget *parent = nullptr);

    void setText(const QString &strIP);
    QString text() const;

protected:
    void paintEvent(QPaintEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *ev) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    int getIndex(QLineEdit *pEdit);
    bool isTextValid(const QString &strIP);

signals:
    void ipChanged(const QString &ip);

private slots:
    void onTextEdit(const QString &str);

private:
    QList<QLineEdit *> m_lineEditList;

    int m_marginLeft = 15;
};

#endif // LINEEDITIP_H

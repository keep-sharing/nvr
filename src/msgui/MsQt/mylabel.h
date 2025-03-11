#ifndef MYLABEL_H
#define MYLABEL_H

#include <QLabel>

class MyLabel : public QLabel
{
    Q_OBJECT
public:
    explicit MyLabel(QWidget *parent = 0);

    void setTranslatableText(const QString &key, const QString &defaultValue);
    void retranslate();

    void setElidedText(const QString &text);
    void clear();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void resetElidedText();

signals:

public slots:

private:
    QString m_textKey;
    QString m_defaultValue;

    QString m_wholeText;
};

#endif // MYLABEL_H

#ifndef TOOLBUTTONFACE_H
#define TOOLBUTTONFACE_H

#include <QToolButton>

class ToolButtonFace : public QToolButton
{
    Q_OBJECT
public:
    explicit ToolButtonFace(QWidget *parent = nullptr);

    void setElidedText(const QString &text);
    QString wholeText() const;

protected:
    void mousePressEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;

signals:

private:
    QString m_wholeText;
};

#endif // TOOLBUTTONFACE_H

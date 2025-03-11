#ifndef ANIMATETOASTITEM_H
#define ANIMATETOASTITEM_H

#include <QWidget>

namespace Ui {
class AnimateToastItem;
}

class AnimateToastItem : public QWidget
{
    Q_OBJECT

public:
    explicit AnimateToastItem(QWidget *parent = nullptr);
    ~AnimateToastItem();

    void setBackgroundColor(const QColor &color);
    void setItemText(const QString &key, const QString &value);
    void setItemTextColor(const QColor &color);

protected:
    void paintEvent(QPaintEvent *) override;

private:
    Ui::AnimateToastItem *ui;

    QColor m_backgroundColor;
};

#endif // ANIMATETOASTITEM_H

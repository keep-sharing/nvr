#ifndef MYCHECKBOX_H
#define MYCHECKBOX_H

#include <QCheckBox>

class MyCheckBox : public QCheckBox
{
    Q_OBJECT
public:
    explicit MyCheckBox(QWidget *parent = 0);

    void setCheckState(Qt::CheckState state);
    void setChecked(bool checked);

    void reset();
    void updateStyle();

protected:
    bool event(QEvent *e) override;
    bool eventFilter(QObject *obj, QEvent *e) override;

signals:
    void checkStateSet(int state);

public slots:
};

#endif // MYCHECKBOX_H

#ifndef MYTEXTEDIT_H
#define MYTEXTEDIT_H

#include <QTextEdit>

class MyTextEdit : public QTextEdit {
    Q_OBJECT
public:
    explicit MyTextEdit(QWidget *parent = nullptr);

protected:
    void focusOutEvent(QFocusEvent *event) override;

signals:
    void editingFinished();
public slots:
};

#endif // MYTEXTEDIT_H

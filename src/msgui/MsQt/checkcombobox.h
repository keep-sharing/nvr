#ifndef CHECKCOMBOBOX_H
#define CHECKCOMBOBOX_H

#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QListWidget>
#include <QMouseEvent>

class CheckComboBox : public QComboBox {
    Q_OBJECT
public:
    explicit CheckComboBox(QWidget *parent = nullptr);

    void addItem(const QString &text, int data);
    void delItem();
    void setLineEditText(const QString &text);
    void checkedAll();
    void clearCheck();

    QList<int> itemCheckedList();

    void hidePopup() override;
    void showPopup() override;

protected:
    bool eventFilter(QObject *, QEvent *) override;

signals:

public slots:

private slots:
    void onCheckedChanged(int state);
    void onCheckedClick();
    void onTextChanged(const QString &str);

private:
    QListWidget *m_listWidget;
    QLineEdit *m_lineEdit;
    QString m_strSelected;
    bool m_isPopupShow = false;
    bool m_mousePress = false;
};

#endif // CHECKCOMBOBOX_H

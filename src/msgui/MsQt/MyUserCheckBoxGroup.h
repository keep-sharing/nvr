#ifndef MYUSERCHECKBOXGROUP_H
#define MYUSERCHECKBOXGROUP_H

#include <QWidget>
#include <checkboxgroup.h>
#include "channelcheckbox.h"

namespace Ui {
class MyUserCheckBoxGroup;
}

class MyUserCheckBoxGroup : public QWidget
{
    Q_OBJECT

public:
    explicit MyUserCheckBoxGroup(QWidget *parent = nullptr);
    ~MyUserCheckBoxGroup();
    void setCount(int allCount, int columnCount = 8);
    void setCheckBoxStyle();
    void setCheckBoxUserStyle();
    void clearCheck();

    bool hasChannelSelected();

    void setCheckedFromString(const QString &text);
    void setCheckedFromInt(const quint32 value);
    void setAllChecked();
    QList<bool> checkStateList();

    void setIndexEnabled(int index, bool enable);
    QList<int> checkedList(bool containsCurrent = true) const;
    QList<int> checkedList(int maxChecked) const;
    QString checkedMask() const;
    QString checkedMask(int maxChecked) const;
    quint64 checkedFlags(bool containsCurrent = true) const;
    quint64 checkedFlags(int maxChecked) const;

    void setAllButtonText(QString text);

signals:
    void checkBoxClicked();
    void checkBoxClicked(int channel, bool checked);

public slots:
    void onLanguageChanged();
    void onCheckBoxClicked();
    void on_checkBoxAll_clicked(bool checked);

private slots:
    void paintEvent(QPaintEvent *) override;

private:
    Ui::MyUserCheckBoxGroup *ui;
    int m_columnCount = 8;
    QList<ChannelCheckBox *> m_checkBoxList;
};

#endif // MYUSERCHECKBOXGROUP_H

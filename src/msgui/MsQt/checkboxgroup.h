#ifndef CHECKBOXGROUP_H
#define CHECKBOXGROUP_H

#include <QWidget>
#include "channelcheckbox.h"


namespace Ui {
class CheckBoxGroup;
}

class CheckBoxGroup : public QWidget
{
    Q_OBJECT

public:
    explicit CheckBoxGroup(QWidget *parent = 0);
    ~CheckBoxGroup();

    void setCount(int allCount, int columnCount = 8);
    void setCountFromChannelName(int allCount, int columnCount = 8);
    void setCountFromPosName(int allCount, int columnCount = 8);
    void setCountFromGroupName(int allCount, int columnCount = 8);
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
    void setCheckBoxTest(QStringList &textList);
    void clearCheckBoxList();

signals:
    void checkBoxClicked();
    void checkBoxClicked(int channel, bool checked);

public slots:
    void onLanguageChanged();

private slots:
    void onCheckBoxClicked();
    void on_checkBoxAll_clicked(bool checked);

private:
    Ui::CheckBoxGroup *ui;

    int m_columnCount = 8;
    QList<ChannelCheckBox *> m_checkBoxList;
};

#endif // CHECKBOXGROUP_H

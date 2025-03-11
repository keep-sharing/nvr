#ifndef MYCOMBOBOXQUESTION_H
#define MYCOMBOBOXQUESTION_H

#include "combobox.h"
#include "mylineedit.h"

class MyComboBoxQuestion : public ComboBox
{
    Q_OBJECT
public:
    explicit MyComboBoxQuestion(QWidget *parent = nullptr);

    void setCustomText(const QString &text);

protected:
    virtual QString lineEditStyleSheet() const;

signals:

public slots:
    void onLanguageChanged();

private slots:
    void onCurrentIndexChanged(int index);

private:

};

#endif // MYCOMBOBOXQUESTION_H

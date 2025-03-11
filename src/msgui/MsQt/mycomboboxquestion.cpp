#include "mycomboboxquestion.h"
#include <QLineEdit>
#include "MsLanguage.h"
#include "MsDevice.h"

MyComboBoxQuestion::MyComboBoxQuestion(QWidget *parent) :
    ComboBox(parent)
{
    addItem(GET_TEXT("WIZARD/11045", "What's your father's name?"), 0);
    addItem(GET_TEXT("WIZARD/11046", "What's your favorite sport?"), 1);
    addItem(GET_TEXT("WIZARD/11047", "What's your mother's name?"), 2);
    addItem(GET_TEXT("WIZARD/11048", "What's your mobile number?"), 3);
    addItem(GET_TEXT("WIZARD/11049", "What's your first pet's name?"), 4);
    addItem(GET_TEXT("WIZARD/11050", "What's your favorite book?"), 5);
    addItem(GET_TEXT("WIZARD/11051", "What's your favorite game?"), 6);
    addItem(GET_TEXT("WIZARD/11052", "What's your favorite food?"), 7);
    addItem(GET_TEXT("WIZARD/11053", "What's your lucky number?"), 8);
    addItem(GET_TEXT("WIZARD/11054", "What's your favorite color?"), 9);
    addItem(GET_TEXT("WIZARD/11055", "What's your best friend's name?"), 10);
    addItem(GET_TEXT("WIZARD/11056", "Where did you go on your first trip?"), 11);
    addItem(GET_TEXT("WIZARD/11057", "Customized Question"), 12);
    connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(onCurrentIndexChanged(int)));
}

void MyComboBoxQuestion::setCustomText(const QString &text)
{
    setCurrentIndexFromData(12);

    QLineEdit *edit = lineEdit();
    if (edit)
    {
        edit->setText(text);
    }
}

QString MyComboBoxQuestion::lineEditStyleSheet() const
{
    QString text = QString("QLineEdit\
                                {\
                                min-height: 28px;\
                                background: transparent;\
                                color: #FFFFFF;\
                                padding-left: 0px;\
                                padding-right: 0px;\
                                border: 0px solid #b9b9b9;\
                                }");
    return text;
}

void MyComboBoxQuestion::onLanguageChanged()
{
    setItemText(0, GET_TEXT("WIZARD/11045", "What's your father's name?"));
    setItemText(1, GET_TEXT("WIZARD/11046", "What's your favorite sport?"));
    setItemText(2, GET_TEXT("WIZARD/11047", "What's your mother's name?"));
    setItemText(3, GET_TEXT("WIZARD/11048", "What's your mobile number?"));
    setItemText(4, GET_TEXT("WIZARD/11049", "What's your first pet's name?"));
    setItemText(5, GET_TEXT("WIZARD/11050", "What's your favorite book?"));
    setItemText(6, GET_TEXT("WIZARD/11051", "What's your favorite game?"));
    setItemText(7, GET_TEXT("WIZARD/11052", "What's your favorite food?"));
    setItemText(8, GET_TEXT("WIZARD/11053", "What's your lucky number?"));
    setItemText(9, GET_TEXT("WIZARD/11054", "What's your favorite color?"));
    setItemText(10, GET_TEXT("WIZARD/11055", "What's your best friend's name?"));
    setItemText(11, GET_TEXT("WIZARD/11056", "Where did you go on your first trip?"));
    setItemText(12, GET_TEXT("WIZARD/11057", "Customized Question"));
}

void MyComboBoxQuestion::onCurrentIndexChanged(int index)
{
    if (index == SQA_CUSTOMIZED_NO)
    {
        setEditable(true);
        clearEditText();
        QLineEdit *edit = lineEdit();
        if (edit)
        {
            edit->setMaxLength(MAX_QUESTION_CHAR);
            edit->setPlaceholderText(GET_TEXT("WIZARD/11057", "Customized Question"));
            edit->setStyleSheet(lineEditStyleSheet());
        }
    }
    else
    {
        setEditable(false);
    }
}

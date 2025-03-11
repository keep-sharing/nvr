#include "checkcombobox.h"
#include "MyCheckBox.h"
#include <QEvent>
#include <QMouseEvent>
#include <QtDebug>

#define SUB_SELECT_ALL 10

CheckComboBox::CheckComboBox(QWidget *parent)
    : QComboBox(parent)
{
    m_listWidget = new QListWidget(this);

    setModel(m_listWidget->model());
    setView(m_listWidget);
    m_lineEdit = new QLineEdit(this);
    m_lineEdit->setReadOnly(true);
    m_lineEdit->setStyleSheet("padding-left: 20px; border: 0px; background-color: #E1E1E1;");
    setLineEdit(m_lineEdit);
    connect(m_lineEdit, SIGNAL(textChanged(QString)), this, SLOT(onTextChanged(QString)));
    m_lineEdit->installEventFilter(this);

    setProperty("ShowKeyboard", -12345);
}

void CheckComboBox::delItem()
{
    int counter = m_listWidget->count();
    QListWidgetItem *item;
    for (int i = 0; i < counter; i++) {
        item = m_listWidget->takeItem(0);
        delete item;
    }

    m_strSelected.clear();
    m_lineEdit->clear();
    return;
}

void CheckComboBox::setLineEditText(const QString &text)
{
    m_strSelected.clear();
    m_strSelected.append(text);
    m_lineEdit->setText(m_strSelected);
}

void CheckComboBox::checkedAll()
{
    for (int i = 0; i < m_listWidget->count(); ++i) {
        QListWidgetItem *item = m_listWidget->item(i);
        QCheckBox *checkBox = (QCheckBox *)m_listWidget->itemWidget(item);
        checkBox->setCheckState(Qt::Checked);
    }
}

void CheckComboBox::clearCheck()
{
    for (int i = 0; i < m_listWidget->count(); ++i) {
        QListWidgetItem *item = m_listWidget->item(i);
        QCheckBox *checkBox = (QCheckBox *)m_listWidget->itemWidget(item);
        checkBox->setCheckState(Qt::Unchecked);
    }
}

void CheckComboBox::addItem(const QString &text, int data)
{
    QListWidgetItem *item = new QListWidgetItem(m_listWidget);
    item->setData(Qt::UserRole, data);
    m_listWidget->addItem(item);

    MyCheckBox *checkBox = new MyCheckBox(this);
    checkBox->setCheckState(Qt::Checked);
    connect(checkBox, SIGNAL(clicked()), this, SLOT(onCheckedClick()));
    checkBox->setText(text);

    m_listWidget->setItemWidget(item, checkBox);
}

QList<int> CheckComboBox::itemCheckedList()
{
    QList<int> checkedList;
    for (int i = 0; i < m_listWidget->count(); ++i) {
        QListWidgetItem *item = m_listWidget->item(i);
        QCheckBox *checkBox = (QCheckBox *)m_listWidget->itemWidget(item);
        if (checkBox->isChecked()) {
            checkedList.append(item->data(Qt::UserRole).toInt());
        }
    }
    return checkedList;
}

void CheckComboBox::hidePopup()
{
    QPoint pos = mapToGlobal(m_lineEdit->pos());
    QRect rc(pos.x(), pos.y(), m_lineEdit->width(), m_lineEdit->height());

    QPoint viewPos = m_listWidget->viewport()->mapFromGlobal(QCursor::pos());
    QRect viewRc = m_listWidget->viewport()->rect();

    if (rc.contains(QCursor::pos()) && m_isPopupShow && m_mousePress) {
        m_mousePress = false;
    } else if (viewRc.contains(viewPos)) {

    } else {
        m_isPopupShow = false;
        QComboBox::hidePopup();
    }
}

void CheckComboBox::showPopup()
{
    m_isPopupShow = true;
    QComboBox::showPopup();
}

bool CheckComboBox::eventFilter(QObject *obj, QEvent *e)
{
    if (obj == m_lineEdit) {
        switch (e->type()) {
        case QEvent::MouseButtonPress:
            if (m_isPopupShow) {
                hidePopup();
            } else {
                showPopup();
                m_mousePress = true;
            }
            break;
        case QEvent::MouseButtonDblClick:
            hidePopup();
            return true;
        default:
            break;
        }
    }
    return QComboBox::eventFilter(obj, e);
}

void CheckComboBox::onCheckedClick()
{
    m_strSelected.clear();
    bool allState = false;

    QCheckBox *senderCheckBox = static_cast<QCheckBox *>(sender());

    if (m_listWidget->count() <= 0)
        return;

    QListWidgetItem *item = m_listWidget->item(0);
    QCheckBox *checkBox = (QCheckBox *)m_listWidget->itemWidget(item);
    QString allText = checkBox->text();
    if (senderCheckBox == checkBox) {
        //first all
        if (checkBox->isChecked()) {
            allState = true;
        } else {
            allText = "";
        }

        for (int i = 1; i < m_listWidget->count(); ++i) {
            item = m_listWidget->item(i);
            checkBox = (QCheckBox *)m_listWidget->itemWidget(item);
            if (allState)
                checkBox->setCheckState(Qt::Checked);
            else
                checkBox->setCheckState(Qt::Unchecked);
        }

        setLineEditText(allText);

        return;
    }

    bool allChecked = true;
    for (int i = 1; i < m_listWidget->count(); ++i) {
        item = m_listWidget->item(i);
        checkBox = (QCheckBox *)m_listWidget->itemWidget(item);

        if (checkBox->isChecked()) {
            m_strSelected.append(checkBox->text() + ";");
        } else {
            allChecked = false;
        }
    }

    if (allChecked) {
        item = m_listWidget->item(0);
        checkBox = (QCheckBox *)m_listWidget->itemWidget(item);
        checkBox->setCheckState(Qt::Checked);
        setLineEditText(allText);
        return;
    } else {
        item = m_listWidget->item(0);
        checkBox = (QCheckBox *)m_listWidget->itemWidget(item);
        checkBox->setCheckState(Qt::Unchecked);
    }

    if (m_strSelected.endsWith(";")) {
        m_strSelected.chop(1);
    }

    if (m_strSelected.isEmpty()) {
        m_lineEdit->clear();
    } else {
        m_lineEdit->setText(m_strSelected);
    }
}

void CheckComboBox::onCheckedChanged(int state)
{
    Q_UNUSED(state)
    m_strSelected.clear();
    bool allState = false;
    bool allSelect = false;

    QCheckBox *senderCheckBox = static_cast<QCheckBox *>(sender());
    for (int i = 0; i < m_listWidget->count(); ++i) {
        QListWidgetItem *item = m_listWidget->item(i);
        QCheckBox *checkBox = (QCheckBox *)m_listWidget->itemWidget(item);

        if (senderCheckBox == checkBox && item->data(Qt::UserRole).toInt() == SUB_SELECT_ALL) {
            //check all
            if (checkBox->isChecked()) {
                allState = true;
                m_strSelected.append(checkBox->text() + ";");
            }
            allSelect = true;
        }

        if (allSelect) {
            if (allState)
                checkBox->setCheckState(Qt::Checked);
            else
                checkBox->setCheckState(Qt::Unchecked);
        } else if (checkBox->isChecked()) {
            if (item->data(Qt::UserRole).toInt() == SUB_SELECT_ALL)
                ;
            else
                m_strSelected.append(checkBox->text() + ";");
        }
    }

    if (m_strSelected.endsWith(";")) {
        m_strSelected.chop(1);
    }

    if (m_strSelected.isEmpty()) {
        m_lineEdit->clear();
    } else {
        m_lineEdit->setText(m_strSelected);
    }
}

void CheckComboBox::onTextChanged(const QString &str)
{
    Q_UNUSED(str)
    m_lineEdit->setText(m_strSelected);
}

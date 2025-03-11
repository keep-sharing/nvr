#ifndef ABSTRACTTESTITEM_H
#define ABSTRACTTESTITEM_H

#include <QWidget>

class AbstractTestItem : public QWidget
{
    Q_OBJECT

    enum TEST_STATE {
        TEST_NONE,
        TEST_PASS,
        TEST_NOT_PASS
    };

public:
    explicit AbstractTestItem(QWidget *parent = nullptr);

    virtual void setModuleName(const QString &text) = 0;
    QString moduleName() const;

    virtual bool isChecked() const;
    virtual void setChecked(bool newChecked);

    void setSelected(bool newSelected);
    void setPassed(bool pass);

    virtual void startTest() = 0;

protected:
    void paintEvent(QPaintEvent *) override;

signals:
    void checkChanged(bool checked);
    void itemStarted(AbstractTestItem *);

protected slots:

protected:
    QString m_moduleName;

    bool m_isChecked = false;
    TEST_STATE m_testState = TEST_NONE;

    bool m_isSelected = false;
};

#endif // ABSTRACTTESTITEM_H

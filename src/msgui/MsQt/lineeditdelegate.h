#ifndef LINEEDITDELEGATE_H
#define LINEEDITDELEGATE_H

#include <QLineEdit>

class LineEditDelegate : public QLineEdit {
    Q_OBJECT

public:
    explicit LineEditDelegate(QWidget *parent = nullptr);
    ~LineEditDelegate() override;

    int row() const;
    void setRow(int row);

    int column() const;
    void setColumn(int column);

    QString oldText() const;
    void setOldText(const QString &oldText);

    QString newText() const;
    void setNewText(const QString &newText);

protected:
    void keyPressEvent(QKeyEvent *) override;

signals:
    void sigEditintFinished(int row, int column, const QString &oldText, const QString &newText);

private slots:
    void onEditingFinished();

private:
    int m_row = 0;
    int m_column = 0;
    QString m_oldText;
    QString m_newText;
};

#endif // LINEEDITDELEGATE_H

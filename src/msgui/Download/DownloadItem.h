#ifndef DOWNLOADITEM_H
#define DOWNLOADITEM_H

#include <QWidget>

namespace Ui {
class DownloadItem;
}

class DownloadItem : public QWidget
{
    Q_OBJECT

public:
    explicit DownloadItem(QWidget *parent = 0);
    ~DownloadItem();

    void initializeData(quint32 id, const QString &name);
    void setProgress(int value);
    void setCompleted();
    bool isCompleted();

    void setErrorVisible(bool visible);
    bool hasError() const;

    quint32 id() const;

    void setRow(int row);
    int row() const;

signals:
    void deleted(quint32 id);

private slots:
    void on_toolButtonClose_clicked();

private:
    Ui::DownloadItem *ui;

    quint32 m_id = 0;

    int m_row = 0;
    bool m_showError = false;
    bool m_isCompleted = false;
};

#endif // DOWNLOADITEM_H

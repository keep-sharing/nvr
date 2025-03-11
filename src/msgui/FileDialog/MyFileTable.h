#ifndef MYFILETABLE_H
#define MYFILETABLE_H

#include <QTableView>
#include <QFileInfo>
#include "headerview.h"

class QLabel;
class MyFileModel;
class MyFileSortModel;
class QFileSystemWatcher;
class MyFileWatcher;

class MyFileTable : public QTableView
{
    Q_OBJECT
public:
    explicit MyFileTable(QWidget *parent = 0);
    ~MyFileTable();

    void setRootIndexPath(const QString &path);
    QString currentPath();
    QString selectedFilePath();

    void setErrorString(const QString &strError);

    void setFixedSize();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private:
    void setWatchPath(const QString &path);
    void clearWatchPath();

signals:
    void currentFileInfo(const QFileInfo &fileInfo);
    void pathChanged(const QString &path);

public slots:
    void onLanguageChanged();

private slots:
    void onItemClicked(const QModelIndex &index);
    void onItemDoubleClicked(const QModelIndex &index);
    void onDirectoryChanged(const QString &path);

private:
    HeaderView *m_headerView;
    MyFileModel *m_fileModel;
    MyFileSortModel *m_sortModel;

    QFileSystemWatcher *m_fileWatcher = nullptr;

    QString m_currentPath;

    QLabel *m_labelError = nullptr;
};

#endif // MYFILETABLE_H

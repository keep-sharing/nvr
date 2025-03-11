#include "MyFileTable.h"
#include <QDir>
#include <QFileInfo>
#include <QHeaderView>
#include <QFileSystemWatcher>
#include <QScrollBar>
#include <QLabel>
#include <QtDebug>
#include "MyFileModel.h"
#include "MyFileSortModel.h"
#include "MyFileWatcher.h"
#include "MyDebug.h"

MyFileTable::MyFileTable(QWidget *parent) :
    QTableView(parent)
{
    m_headerView = new HeaderView(Qt::Horizontal, this);
    m_headerView->setHighlightSections(false);
    m_headerView->setStretchLastSection(true);
    setHorizontalHeader(m_headerView);

    verticalHeader()->setVisible(false);

    m_fileModel = new MyFileModel(this);
    m_sortModel = new MyFileSortModel(this);
    m_sortModel->setSourceModel(m_fileModel);
    setModel(m_sortModel);
    setSortingEnabled(true);
    sortByColumn(MyFileModel::ColumnName, Qt::AscendingOrder);

    setEditTriggers(NoEditTriggers);
    setSelectionMode(QTableView::SingleSelection);
    setSelectionBehavior(QTableView::SelectRows);
    setAlternatingRowColors(true);

    horizontalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);

    setIconSize(QSize(24, 24));

    connect(this, SIGNAL(clicked(QModelIndex)), this, SLOT(onItemClicked(QModelIndex)));
    connect(this, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onItemDoubleClicked(QModelIndex)));

    //
    m_fileWatcher = new QFileSystemWatcher(this);
    //connect(m_fileWatcher, SIGNAL(directoryChanged(QString)), this, SLOT(onDirectoryChanged(QString)));
}

MyFileTable::~MyFileTable()
{

}

void MyFileTable::setRootIndexPath(const QString &path)
{
    if (m_labelError)
    {
        m_labelError->hide();
    }

    m_currentPath = path;
    m_fileModel->setRowCount(0);

    if (path.isEmpty())
    {
        return;
    }

    QDir dir(path);
    QFileInfoList fileInfoList = dir.entryInfoList(QDir::AllEntries | QDir::NoDot, QDir::NoSort);
    m_fileModel->setRowCount(fileInfoList.size());
    for (int i = 0; i < fileInfoList.size(); ++i)
    {
        m_fileModel->setFileInfo(i, fileInfoList.at(i));
    }

    //clearWatchPath();
    //setWatchPath(path);
    emit pathChanged(path);

    m_sortModel->reSort();
}

QString MyFileTable::currentPath()
{
    return m_currentPath;
}

QString MyFileTable::selectedFilePath()
{
    QString path;
    const QModelIndex &index = currentIndex();
    if (index.isValid())
    {
        const QModelIndex &sourceIndex = m_sortModel->mapToSource(index);
        const QFileInfo &fileInfo = m_fileModel->fileInfo(sourceIndex.row());
        path = fileInfo.absoluteFilePath();
    }
    return path;
}

void MyFileTable::setErrorString(const QString &strError)
{
    m_fileModel->setRowCount(0);
    if (!m_labelError)
    {
        m_labelError = new QLabel(this);
        m_labelError->setStyleSheet("color: #000000;background-color: transparent");
        m_labelError->setAlignment(Qt::AlignCenter);
    }
    m_labelError->setGeometry(rect());
    m_labelError->setText(strError);
    m_labelError->show();
}

void MyFileTable::setFixedSize()
{
    int minWidth = (width() - 50) / 6;
    setColumnWidth(MyFileModel::ColumnName, minWidth * 3);
    setColumnWidth(MyFileModel::ColumnSize, minWidth);
    setColumnWidth(MyFileModel::ColumnType, minWidth);
    setColumnWidth(MyFileModel::ColumnDate, minWidth);
}

void MyFileTable::resizeEvent(QResizeEvent *event)
{
    QTableView::resizeEvent(event);
}

void MyFileTable::hideEvent(QHideEvent *event)
{
    clearWatchPath();
    QTableView::hideEvent(event);
}

void MyFileTable::setWatchPath(const QString &path)
{
    qDebug() << "MyFileTable::setWatchPath" << path;
    const QStringList &list = m_fileWatcher->directories();
    if (!list.contains(path))
    {
        m_fileWatcher->addPath(path);
    }
}

void MyFileTable::clearWatchPath()
{
    qDebug() << "MyFileTable::clearWatchPath";
    const QStringList &list = m_fileWatcher->directories();
    if (!list.isEmpty())
    {
        m_fileWatcher->removePaths(list);
    }
}

void MyFileTable::onLanguageChanged()
{
    m_fileModel->onLanguageChanged();
}

void MyFileTable::onItemClicked(const QModelIndex &index)
{
    const QModelIndex &sourceIndex = m_sortModel->mapToSource(index);
    const QFileInfo &fileInfo = m_fileModel->fileInfo(sourceIndex.row());
    emit currentFileInfo(fileInfo);
}

void MyFileTable::onItemDoubleClicked(const QModelIndex &index)
{
    const QModelIndex &sourceIndex = m_sortModel->mapToSource(index);
    const QFileInfo &fileInfo = m_fileModel->fileInfo(sourceIndex.row());
    if (fileInfo.isDir())
    {
        QString strPath = fileInfo.absoluteFilePath();
        if (strPath != QString("/media"))
        {
            setRootIndexPath(strPath);
        }
    }
}

void MyFileTable::onDirectoryChanged(const QString &path)
{
    qDebug() << "MyFileTable::onDirectoryChanged" << path;
    if (path == m_currentPath)
    {
        setRootIndexPath(m_currentPath);
    }
}


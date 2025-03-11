#include "MyFileWatcher.h"
#include <QDir>
#include <QElapsedTimer>
#include <QtDebug>

MyFileWatcher::MyFileWatcher(QObject *parent) : QObject(parent)
{
    moveToThread(&m_thread);
    connect(&m_thread, SIGNAL(started()), this, SLOT(onThreadStart()));
    connect(&m_thread, SIGNAL(finished()), this, SLOT(onThreadFinish()));
    m_thread.setObjectName("Qt-MyFileWatcher");
    m_thread.start();
}

MyFileWatcher::~MyFileWatcher()
{
    qDebug() << QString("~MyFileWatcher()");
}

void MyFileWatcher::stopThread()
{
    m_thread.quit();
    m_thread.wait();
}

void MyFileWatcher::setPath(const QString &path)
{
    QMetaObject::invokeMethod(this, "onSetPath", Q_ARG(QString, path));
}

void MyFileWatcher::clearPath()
{
    QMetaObject::invokeMethod(this, "onClearPath");
}

void MyFileWatcher::onThreadStart()
{
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onFileWatcherTimer()));
    m_timer->setInterval(1000);
}

void MyFileWatcher::onThreadFinish()
{
    m_timer->stop();
}

void MyFileWatcher::onSetPath(const QString &path)
{
    if (path.isEmpty())
    {
        return;
    }

    if (path == m_watchPath)
    {
        return;
    }

    m_watchPath = path;
    m_dirInfo.setDir(path);

    if (!m_timer->isActive())
    {
        m_timer->start();
    }
}

void MyFileWatcher::onClearPath()
{
    m_watchPath.clear();
    m_timer->stop();
}

void MyFileWatcher::onFileWatcherTimer()
{
    DirInfo tempDirInfo(m_watchPath);
    if (tempDirInfo == m_dirInfo)
    {

    }
    else
    {
        emit directoryChanged(m_watchPath);
        m_dirInfo = tempDirInfo;
    }
}

/***************************
 * class DirInfo
 **************************/
DirInfo::DirInfo()
{

}

DirInfo::DirInfo(const QString &dir)
{
    setDir(dir);
}

void DirInfo::setDir(const QString &dir)
{
    entryList.clear();
    fileModifiedMap.clear();;

    QDir d(dir);
    const QFileInfoList fileInfoList = d.entryInfoList();
    for (int i = 0; i < fileInfoList.size(); ++i)
    {
        const QFileInfo &fileInfo = fileInfoList.at(i);
        entryList.append(fileInfo.fileName());
        if (fileInfo.isFile())
        {
            fileModifiedMap.insert(fileInfo.fileName(), fileInfo.lastModified());
        }
    }
}

bool DirInfo::operator ==(const DirInfo &other) const
{
    if (entryList != other.entryList)
    {
        return false;
    }
    if (fileModifiedMap != other.fileModifiedMap)
    {
        return false;
    }
    return true;
}

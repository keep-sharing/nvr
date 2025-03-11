#include "anprthread.h"
#include <QFile>
#include <QtDebug>
#include "MsLanguage.h"

AnprThread::AnprThread(QObject *parent) :
    QObject(parent)
{
    moveToThread(&m_thread);
    m_thread.setObjectName("Qt-AnprThread");
    m_thread.start();
}

void AnprThread::stopThread()
{
    m_thread.quit();
    m_thread.wait();
}

void AnprThread::setImportRepeatMode(AnprThread::RepeatMode mode)
{
    m_mutex.lock();
    m_importRepeatMode = mode;
    m_wait.wakeAll();
    m_mutex.unlock();
}

void AnprThread::importAnprList(const QString &filePath)
{
    QMetaObject::invokeMethod(this, "onImportAnprList", Q_ARG(QString, filePath));
}

void AnprThread::deleteAnprList(const QList<anpr_list> &list)
{
    m_deleteAnprList = list;
    QMetaObject::invokeMethod(this, "onDeleteAnprList");
}

void AnprThread::stopOperate()
{
    qDebug() << QString("AnprThread::stopOperate");
    QMutexLocker locker(&m_mutex);
    m_isStop = true;
}

bool AnprThread::isStop()
{
    QMutexLocker locker(&m_mutex);
    return m_isStop;
}

void AnprThread::onImportAnprList(const QString &filePath)
{
    qDebug() << QString("AnprThread::onImportAnprList, begin");

    QFile file(filePath);
    if (!file.open(QFile::ReadOnly))
    {
        qWarning() << QString("AnprThread::onImportAnprList, %1").arg(file.errorString());
        emit importError(QString("%1").arg(file.errorString()));
        return;
    }

    QTextStream in(&file);
    //格式检查
    QString strHeader = in.readLine();
    if (strHeader != QString("Type,Plate"))
    {
        qWarning() << QString("AnprThread::onImportAnprList, File error.");
        emit importError(GET_TEXT("ANPR/103050", "File error"));
        return;
    }

    //
    QString strLine;
    QStringList plateList;
    do {
        strLine = in.readLine();
        if (!strLine.isEmpty())
        {
            plateList.append(strLine);
        }
    }
    while (!strLine.isNull());

    //
    anpr_list *anpr_list_array = nullptr;
    int anpr_list_count = 0;
    read_anpr_lists(SQLITE_ANPR_NAME, &anpr_list_array, &anpr_list_count);
    QMap<QString, QString> anprMap;
    for (int i = 0; i < anpr_list_count; ++i)
    {
        const anpr_list &anpr_info = anpr_list_array[i];
        anprMap.insert(QString(anpr_info.plate), QString(anpr_info.type));
    }
    release_anpr_lists(&anpr_list_array);

    //
    emit importProgress(0);
    m_isStop = false;
    int succeedCount = 0;
    int failedCount = 0;
    bool isReachLimit = false;
    m_importRepeatMode = ModeNone;
    QStringList errorList;
    for (int i = 0; i < plateList.size(); ++i)
    {
        if (isStop())
        {
            qDebug() << QString("AnprThread::onImportAnprList, cancel");
            failedCount += (plateList.size() - i);
            break;
        }

        //
        if (anprMap.size() >= MAX_ANPR_LIST_COUNT)
        {
            failedCount += (plateList.size() - i);
            isReachLimit = true;
            break;
        }
        //
        QString text = plateList.at(i);
        QRegExp rx("(.+),(.+)");
        if (rx.indexIn(text) != -1)
        {
            const QString &strType = rx.cap(1);
            const QString &strPlate = rx.cap(2);

            if (strType != QString(PARAM_MS_ANPR_TYPE_BLACK) && strType != QString(PARAM_MS_ANPR_TYPE_WHITE))
            {
                errorList.append(strLine);
                failedCount++;
                continue;
            }
            if (strPlate.contains(" "))
            {
                errorList.append(strLine);
                failedCount++;
                continue;
            }

            //
            if (anprMap.contains(strPlate))
            {
                if (m_importRepeatMode == ModeNone)
                {
                    emit importDealRepeat();
                    m_mutex.lock();
                    m_wait.wait(&m_mutex);
                    m_mutex.unlock();
                }
                switch (m_importRepeatMode)
                {
                case ModeIgnore:
                    failedCount++;
                    break;
                case ModeReplace:
                    anprMap.insert(strPlate, strType);
                    succeedCount++;
                    break;
                default:
                    break;
                }
            }
            else
            {
                anprMap.insert(strPlate, strType);
                succeedCount++;
            }
        }
        else
        {
            if (!strLine.isEmpty())
            {
                errorList.append(strLine);
                failedCount++;
            }
        }
    }

    //
    anpr_list *anpr_list_array_write = new anpr_list[MAX_ANPR_LIST_COUNT];
    memset(anpr_list_array_write, 0, sizeof(anpr_list) * MAX_ANPR_LIST_COUNT);
    int index = 0;
    for (auto iter = anprMap.constBegin(); iter != anprMap.constEnd(); ++iter)
    {
        QString plate = iter.key();
        QString type = iter.value();
        anpr_list &anpr_info = anpr_list_array_write[index];
        snprintf(anpr_info.plate, sizeof(anpr_info.plate), "%s", plate.toStdString().c_str());
        snprintf(anpr_info.type, sizeof(anpr_info.type), "%s", type.toStdString().c_str());
        index++;
    }
    write_anpr_lists(SQLITE_ANPR_NAME, anpr_list_array_write, anprMap.size());
    delete[] anpr_list_array_write;
    //
    emit importProgress(100);
    emit importResult(succeedCount, failedCount, errorList.size(), isReachLimit, isStop());

    qDebug() << QString("AnprThread::onImportAnprList, end");
}

void AnprThread::onDeleteAnprList()
{
    qDebug() << QString("AnprThread::onDeleteAnprList, begin");

    m_isStop = false;
    emit deleteProgress(0);
    anpr_list *anpr_info_list = new anpr_list[m_deleteAnprList.size()];
    memset(anpr_info_list, 0, sizeof(anpr_list) * m_deleteAnprList.size());
    for (int i = 0; i < m_deleteAnprList.size(); ++i)
    {
        anpr_list &anpr_info = anpr_info_list[i];
        memcpy(&anpr_info, &m_deleteAnprList.at(i), sizeof(anpr_list));
    }
    delete_anpr_lists(SQLITE_ANPR_NAME, anpr_info_list, m_deleteAnprList.size());
    delete[] anpr_info_list;
    emit deleteProgress(100);
    emit deleteResult();

    qDebug() << QString("AnprThread::onDeleteAnprList, end");
}

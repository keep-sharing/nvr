#include "languagereader.h"
#include "MyDebug.h"
#include <QRegExp>
#include <QtDebug>

LanguageReader::LanguageReader(const QFileInfo &fileInfo, QObject *parent)
    : QObject(parent)
{
    setLanguage(fileInfo);
}

LanguageReader::LanguageReader(const QString &filePath, QObject *parent)
    : QObject(parent)
{
    setLanguage(QFileInfo(filePath));
}

void LanguageReader::setLanguage(const QFileInfo &fileInfo)
{
    m_textMap.clear();
    //
    QFile file(fileInfo.absoluteFilePath());
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qWarning() << QString("%1, Open file failed: %2").arg(fileInfo.absoluteFilePath()).arg(file.errorString());
        return;
    }

    QString strGroup;
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString strLine = in.readLine();
        if (strLine.startsWith("#")) {
            continue;
        }
        QRegExp rxGroup("^\\[(.+)\\]$");
        if (rxGroup.indexIn(strLine) != -1) {
            strGroup = rxGroup.cap(1);
            continue;
        }
        QRegExp rxName("Language=(.*)");
        if (rxName.indexIn(strLine) != -1) {
            m_name = rxName.cap(1);
            continue;
        }
        QRegExp rxID("LangID=(.*)");
        if (rxID.indexIn(strLine) != -1) {
            m_id = rxID.cap(1).toInt();
            continue;
        }
        QRegExp rxText("(^\\d+)=(.*)");
        if (rxText.indexIn(strLine) != -1) {
            QString strKey = rxText.cap(1);
            QString strValue = rxText.cap(2);
            QString strCompleteKey = QString("%1/%2").arg(strGroup).arg(strKey);
            m_textMap.insert(strCompleteKey, strValue);
        }
    }
}

int LanguageReader::languageID() const
{
    return m_id;
}

QString LanguageReader::languageName() const
{
    return m_name;
}

QString LanguageReader::value(const QString &key, const QString &defaultValue)
{
    if (m_textMap.contains(key)) {
        return m_textMap.value(key);
    } else {
        return defaultValue;
    }
}

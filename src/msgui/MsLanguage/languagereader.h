#ifndef LANGUAGEREADER_H
#define LANGUAGEREADER_H

#include <QFileInfo>
#include <QMap>
#include <QObject>

class LanguageReader : public QObject {
    Q_OBJECT

public:
    explicit LanguageReader(const QFileInfo &fileInfo, QObject *parent = nullptr);
    explicit LanguageReader(const QString &filePath, QObject *parent = nullptr);

    void setLanguage(const QFileInfo &fileInfo);

    int languageID() const;
    QString languageName() const;

    QString value(const QString &key, const QString &defaultValue);

signals:

public slots:

private:
    int m_id = -1;
    QString m_name;

    QMap<QString, QString> m_textMap;
};

#endif // LANGUAGEREADER_H

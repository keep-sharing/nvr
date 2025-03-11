#ifndef MSLANGUAGE_H
#define MSLANGUAGE_H

#include <QComboBox>
#include <QMap>
#include <QObject>
#include <QSettings>

class LanguageReader;

#define GET_TEXT(x, y) MsLanguage::instance()->value((x), (y))
#define TEMP_TEXT(a) QString(a)

class MsLanguage : public QObject {
    Q_OBJECT

public:
    enum Language {
        LAN_PL = 6
    };

    struct Info {
        QString name;
        QString path;
    };

    explicit MsLanguage(QObject *parent = 0);

    static MsLanguage *instance();
    void initialize();

    //
    void initializeComboBox(QComboBox *comboBox);

    QMap<int, Info> languageMap();
    int currentLanguage();
    void changeLanguage(int id);

    QString value(const QString &key, const QString &defaultValue);

signals:
    void languageChanged();

public slots:

private:
    QMap<int, Info> m_languageMap; //key: id
    int m_currentId = -1;

    LanguageReader *m_currentLanguage = nullptr;
};

#endif // MSLANGUAGE_H

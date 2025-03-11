#include "MsLanguage.h"
#include "MsDevice.h"
#include "MyDebug.h"
#include "languagereader.h"
#include <QApplication>
#include <QDir>
#include <QFontDatabase>
#include <QMutex>

MsLanguage::MsLanguage(QObject *parent)
    : QObject(parent)
{
    QDir dir(":/language/lang");
    QFileInfoList fileInfoList = dir.entryInfoList(QDir::Files);
    for (int i = 0; i < fileInfoList.size(); ++i) {
        const QFileInfo &fileInfo = fileInfoList.at(i);

        LanguageReader reader(fileInfo);

        MsLanguage::Info info;
        info.name = reader.languageName();
        info.path = fileInfo.absoluteFilePath();

        int id = reader.languageID();
        if (m_languageMap.contains(id)) {
            qCritical() << QString("Language id conflict: %1, %2").arg(fileInfo.absoluteFilePath()).arg(m_languageMap.value(id).path);
            continue;
        }
        m_languageMap.insert(id, info);
    }

    //
    QSettings defaultLanguage(GUI_LANG_INI, QSettings::IniFormat);
    defaultLanguage.setIniCodec("UTF-8");
    int id = defaultLanguage.value("language", 0).toInt();
    changeLanguage(id);
}

MsLanguage *MsLanguage::instance()
{
    static MsLanguage self;
    return &self;
}

void MsLanguage::initialize()
{
    //字体
    int fontid = -1;
    switch (currentLanguage()) {
    case 16:
    case 17:
    case 19:
        fontid = QFontDatabase::addApplicationFont("/opt/app/bin/fonts/GothicA1-Regular.ttf");
        break;
    default:
        fontid = QFontDatabase::addApplicationFont("/opt/app/bin/fonts/CenturyGothic.TTF");
        break;
    }
    if (fontid < 0) {
        qDebug() << "Failed to load font, default font will be used.";
        QFont font("Bitstream Vera Sans", 14);
        qApp->setFont(font);
    } else {
        QFont font(QFontDatabase::applicationFontFamilies(fontid).at(0));
        font.setPixelSize(14);
        //font.setWeight(50);
        font.setStyleStrategy(QFont::PreferAntialias);
        qApp->setFont(font);
    }
    qDebug() << "Application Font:" << qApp->font();
}

void MsLanguage::initializeComboBox(QComboBox *comboBox)
{
    comboBox->clear();

    qint32 languageMask = get_param_int(SQLITE_FILE_NAME, PARAM_DEVICE_LANG, 0x7FFFFFFF);

    for (auto iter = m_languageMap.constBegin(); iter != m_languageMap.constEnd(); ++iter) {
        const int &id = iter.key();
        if (languageMask & (1 << id)) {
            const Info &info = iter.value();
            comboBox->addItem(info.name, id);
        }
    }
    comboBox->setCurrentIndex(comboBox->findData(m_currentId));
}

QMap<int, MsLanguage::Info> MsLanguage::languageMap()
{
    return m_languageMap;
}

int MsLanguage::currentLanguage()
{
    return m_currentId;
}

void MsLanguage::changeLanguage(int id)
{
    qDebug() << "====MsLanguage::changeLanguage====";
    qDebug() << "----id:" << id;
    if (!m_languageMap.contains(id)) {
        return;
    }
    if (id == m_currentId) {
        return;
    }
    delete m_currentLanguage;
    m_currentId       = id;
    m_currentLanguage = new LanguageReader(m_languageMap.value(m_currentId).path, this);
    //
    QSettings defaultLanguage(GUI_LANG_INI, QSettings::IniFormat);
    defaultLanguage.setIniCodec("UTF-8");
    defaultLanguage.setValue("language", m_currentId);

    //
#if 0
    int fontid = -1;
    switch (id) {
    case 0: // 英语
        fontid = QFontDatabase::addApplicationFont("/opt/app/bin/fonts/CenturyGothic.TTF");
        break;
    case 1: // 简体中文
    case 2: // 繁体中文
        fontid = QFontDatabase::addApplicationFont("/opt/app/bin/fonts/zh.ttf");
        break;
    case 11: // 日语, ja_JP.lng
        fontid = QFontDatabase::addApplicationFont("/opt/app/bin/fonts/Boku2-Regular.ttf");
        break;
    case 12: // 韩语, ko_KR.lng
        fontid = QFontDatabase::addApplicationFont("/opt/app/bin/fonts/GothicA1-Regular.ttf");
        break;
    case 16: // Hebrew, 希伯来语, iw_IL.lng
        fontid = QFontDatabase::addApplicationFont("/opt/app/bin/fonts/david.ttf");
        break;
    case 17: // Arabic, 阿拉伯语, ar_EG.lng
        fontid = QFontDatabase::addApplicationFont("/opt/app/bin/fonts/DejaVuSansMono.ttf");
        break;
    case 19: // Persian, 波斯语, fa_IR.lng
        fontid = QFontDatabase::addApplicationFont("/opt/app/bin/fonts/DejaVuSansMono.ttf");
        break;
    }
    if (fontid < 0) {
        fontid = QFontDatabase::addApplicationFont("/opt/app/bin/fonts/CenturyGothic.TTF");
    }
    if (fontid >= 0) {
        QFont font(QFontDatabase::applicationFontFamilies(fontid).at(0));
        font.setPixelSize(14);
        //font.setWeight(50);
        font.setStyleStrategy(QFont::PreferAntialias);
        qApp->setFont(font);
    } else {
        qMsWarning() << QString("invalid font, id:%1").arg(id);
    }
#endif

    //语言改变
    emit languageChanged();
}

QString MsLanguage::value(const QString &key, const QString &defaultValue)
{
    QString text;
    if (!m_currentLanguage) {
        return text;
    }
    text = m_currentLanguage->value(key, defaultValue);
    return text;
}

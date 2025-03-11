#include "TargetInfoManager.h"
#include "LogoutChannel.h"
#include "MyDebug.h"
#include "msuser.h"

TargetInfoManager::TargetInfoManager(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<MS_VCA_ALARM>("MS_VCA_ALARM");

#if 0
    //anpr test
    AnprSimulation *simulation = new AnprSimulation(this);
    simulation->start();
#endif

    moveToThread(&m_thread);
    connect(&m_thread, SIGNAL(started()), this, SLOT(onThreadStarted()));
    connect(&m_thread, SIGNAL(finished()), this, SLOT(onThreadFinished()));
    m_thread.setObjectName("Qt-TargetInfoManager");
    m_thread.start();

    m_displayFlags = get_param_int(SQLITE_FILE_NAME, "target_preview_diaplay", 0);

#if 0	
	if(access(LPR_NO_GUI, F_OK) == 0)
	{
		msprintf("no ms_add_lpr_watcher GUI");
		return ;
	}
#endif
}

TargetInfoManager::~TargetInfoManager()
{

    m_thread.quit();
    m_thread.wait();

    //
    for (int i = 0; i < m_tempInfoList.size(); ++i) {
        TargetInfo *info = m_tempInfoList.at(i);
        delete info;
    }
    m_tempInfoList.clear();
    //
    for (int i = 0; i < m_infoList.size(); ++i) {
        TargetInfo *info = m_infoList.at(i);
        delete info;
    }
    m_infoList.clear();
}

void TargetInfoManager::anprCallback(void *msg, void *image)
{
    struct lpr_metadata_info *pMsg = (struct lpr_metadata_info *)msg;
    struct lpr_image_info *pImage = (struct lpr_image_info *)image;

    if (!pMsg) {
        //qWarning() << QString("TargetInfoManager::anprCallback, data is null.");
        return;
    }
    if (!pImage) {
        //qWarning() << QString("TargetInfoManager::anprCallback, image is null.");
        return;
    }

    qMsCDebug("qt_anpr_callback") << QString("channel:%1, time:%2, plate:%3, type:%4, vehicleType:%5")
                                         .arg(pMsg->chnid)
                                         .arg(pMsg->ptime)
                                         .arg(pMsg->plate)
                                         .arg(pMsg->ptype)
                                         .arg(pMsg->vehicleType);

    //
    if (gTargetInfoManager.isReceiveEnable() && gTargetInfoManager.checkDisplayFlag(TargetInfo::TARGET_ANPR)) {
        gTargetInfoManager.setTempAnprInfo(pMsg, pImage);
    }
}

TargetInfoManager &TargetInfoManager::instance()
{
    static TargetInfoManager self;
    return self;
}

void TargetInfoManager::readyToQuit()
{

}

void TargetInfoManager::lock()
{
    m_mutex.lock();
}

bool TargetInfoManager::tryLock()
{
    return m_mutex.tryLock();
}

bool TargetInfoManager::tryLock(int timeout)
{
    return m_mutex.tryLock(timeout);
}

void TargetInfoManager::unlock()
{
    m_mutex.unlock();
}

void TargetInfoManager::setReceiveEnable(bool enable)
{
    m_isReveive = enable;
    if (!m_isReveive) {
        clearTargetInfo();
    }
}

bool TargetInfoManager::isReceiveEnable()
{
    return m_isReveive;
}

void TargetInfoManager::setTempAnprInfo(lpr_metadata_info *pMsg, lpr_image_info *pImage)
{
    if (!gMsUser.hasLiveViewChannelPermission(pMsg->chnid)) {
        return;
    }

    if (LogoutChannel::instance()) {
        if (LogoutChannel::instance()->isLogout() && !LogoutChannel::instance()->isLogoutChannel(pMsg->chnid)) {
            return;
        }
    }

    //
    m_tempMutex.lock();
    while (m_tempInfoList.size() >= MAX_TARGET_ITEM_COUNT) {
        TargetInfo *firstInfo = m_tempInfoList.takeFirst();
        delete firstInfo;
    }
    TargetInfoAnpr *info = new TargetInfoAnpr(pMsg, pImage);
    m_tempInfoList.append(info);
    m_tempMutex.unlock();

    //
    if (!isProcessingTargetInfo()) {
        setIsProcessingTargetInfo(true);
        QMetaObject::invokeMethod(this, "processTargetInfo", Qt::QueuedConnection);
    }
}

void TargetInfoManager::setTempVcaInfo(MS_VCA_ALARM *alarm)
{
    if (!gMsUser.hasLiveViewChannelPermission(alarm->chnid)) {
        return;
    }

    if (LogoutChannel::instance()) {
        if (LogoutChannel::instance()->isLogout() && !LogoutChannel::instance()->isLogoutChannel(alarm->chnid)) {
            return;
        }
    }

    //
    m_tempMutex.lock();
    while (m_tempInfoList.size() >= MAX_TARGET_ITEM_COUNT) {
        TargetInfo *firstInfo = m_tempInfoList.takeFirst();
        delete firstInfo;
    }
    TargetInfoVca *info = new TargetInfoVca(alarm);
    m_tempInfoList.append(info);
    m_tempMutex.unlock();

    //
    if (!isProcessingTargetInfo()) {
        setIsProcessingTargetInfo(true);
        QMetaObject::invokeMethod(this, "processTargetInfo", Qt::QueuedConnection);
    }
}

void TargetInfoManager::setTempFaceInfo(MS_FACE_ALARM *face)
{
    if (!gMsUser.hasLiveViewChannelPermission(face->chnId)) {
        return;
    }

    if (LogoutChannel::instance()) {
        if (LogoutChannel::instance()->isLogout() && !LogoutChannel::instance()->isLogoutChannel(face->chnId)) {
            return;
        }
    }

    //
    m_tempMutex.lock();
    for (int i = 0; i < face->faceCnt; ++i) {
        while (m_tempInfoList.size() >= MAX_TARGET_ITEM_COUNT) {
            TargetInfo *firstInfo = m_tempInfoList.takeFirst();
            delete firstInfo;
        }
        const MS_FACE_IMAGE &faceImage = face->image[i];
        TargetInfoFace *info = new TargetInfoFace(face->chnId, QString(face->time), &faceImage);
        m_tempInfoList.append(info);
    }
    m_tempMutex.unlock();

    //
    if (!isProcessingTargetInfo()) {
        setIsProcessingTargetInfo(true);
        QMetaObject::invokeMethod(this, "processTargetInfo", Qt::QueuedConnection);
    }
}

bool TargetInfoManager::isProcessingTargetInfo()
{
    QMutexLocker locker(&m_mutex);
    return m_isProcessingTargetInfo;
}

void TargetInfoManager::setIsProcessingTargetInfo(bool newIsProcessingTargetInfo)
{
    QMutexLocker locker(&m_mutex);
    m_isProcessingTargetInfo = newIsProcessingTargetInfo;
}

/**
 * @brief TargetInfoManager::getAnprInfo
 * @param index
 * @return
 */
TargetInfo *TargetInfoManager::getTargetInfo(int index) const
{
    if (index < 0 || index >= m_infoList.size()) {
        return nullptr;
    }
    //最新的显示在最上方
    int realIndex = m_infoList.size() - index - 1;
    TargetInfo *info = m_infoList.at(realIndex);
    return info;
}

void TargetInfoManager::clearTargetInfo()
{
    lock();
    for (int i = 0; i < m_infoList.size(); ++i) {
        TargetInfo *info = m_infoList.at(i);
        delete info;
        info = nullptr;
    }
    m_infoList.clear();
    unlock();
    emit infoCleared();
}

void TargetInfoManager::updatePlateType()
{
    qDebug() << QString("TargetInfoManager::updatePlateType");

    lock();
    for (int i = 0; i < m_infoList.size(); ++i) {
        TargetInfo *info = m_infoList.at(i);
        if (info->type() == TargetInfo::TARGET_ANPR) {
            TargetInfoAnpr *anprInfo = static_cast<TargetInfoAnpr *>(info);
            QString plate = anprInfo->licenseString();

            anpr_list anpr_info;
            memset(&anpr_info, 0, sizeof(anpr_list));
            read_anpr_list_plate(SQLITE_ANPR_NAME, &anpr_info, plate.toStdString().c_str());
            if (!QString(anpr_info.type).isEmpty()) {
                anprInfo->setAnprTypeString(QString(anpr_info.type));
            } else {
                anprInfo->setAnprTypeString("Visitor");
            }
        }
    }
    unlock();
    emit infoChanged();
}

void TargetInfoManager::setDisplayFlags(int flags)
{
    lock();
    m_displayFlags = flags;
    unlock();
}

bool TargetInfoManager::checkDisplayFlag(int flag)
{
    QMutexLocker locker(&m_mutex);
    return m_displayFlags & flag;
}

void TargetInfoManager::emitVcaAlarm(MS_VCA_ALARM *alarm)
{
    emit vcaAlarm(*alarm);
}

void TargetInfoManager::onThreadStarted()
{
}

void TargetInfoManager::onThreadFinished()
{
}

void TargetInfoManager::processTargetInfo()
{
    TargetInfo *tempInfo = nullptr;
    m_tempMutex.lock();
    while (!m_tempInfoList.isEmpty()) {
        tempInfo = m_tempInfoList.takeFirst();
        m_tempMutex.unlock();
        //
        tempInfo->makeImage();
        lock();
        while (m_infoList.size() >= MAX_TARGET_ITEM_COUNT) {
            TargetInfo *firstInfo = m_infoList.takeFirst();
            delete firstInfo;
        }
        m_infoList.append(tempInfo);
        unlock();
        emit infoChanged();
        //
        m_tempMutex.lock();
    }
    m_tempMutex.unlock();

    //
    setIsProcessingTargetInfo(false);
}

#ifndef FACEDATA_H
#define FACEDATA_H
#include "MsObject.h"
#include <QEventLoop>

extern "C" {
#include "msg.h"
}

#define gFaceData FaceData::instance()
class FaceData : public MsObject {
    Q_OBJECT
public:
    explicit FaceData();
    ~FaceData();
    static FaceData &instance();

    void processMessage(MessageReceive *message) override;
    void getFaceConfig(int channel);
    bool isFaceConflict();
    void setFaceDisable();

private:
    void ON_RESPONSE_FLAG_GET_FACE_SUPPORT(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_FACE_CONFIG(MessageReceive *message);
    void ON_RESPONSE_FLAG_SET_FACE_CONFIG(MessageReceive *message);

private:
    QEventLoop m_eventLoop;

    bool m_isFaceSupport = false;
    MsFaceConfig m_faceConfig;
};

#endif // FACEDATA_H

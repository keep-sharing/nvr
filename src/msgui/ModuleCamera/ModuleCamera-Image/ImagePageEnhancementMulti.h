#ifndef IMAGEPAGEENHANCEMENTMULTI_H
#define IMAGEPAGEENHANCEMENTMULTI_H

#include "MsCameraModel.h"
#include "abstractimagepage.h"
#include "drawenhancement.h"
namespace Ui {
class ImagePageEnhancementMulti;
}

class ImagePageEnhancementMulti : public AbstractImagePage {
  Q_OBJECT

  enum PowerLineFrequency {
    Frequency_60Hz = 0,
    Frequency_50Hz = 1,
    Frequency_Unknow
  };

public:
  explicit ImagePageEnhancementMulti(QWidget *parent = nullptr);
  ~ImagePageEnhancementMulti();

  void initializeData(int channel) override;
  void processMessage(MessageReceive *message) override;

private:
  void clearSettings();
  void setSettingsEnable(bool enable);

  void initEnhancement();
  void hideBWHAll();

  void ON_RESPONSE_FLAG_GET_FISHEYE_MODE(MessageReceive *message);
  void ON_RESPONSE_FLAG_GET_IPCPARAM(MessageReceive *message);
  void ON_RESPONSE_FLAG_GET_IPCIMAGE_ENHANCEMENT(MessageReceive *message);
  void ON_RESPONSE_FLAG_GET_IPC_COMMON_PARAM(MessageReceive *message);
  void ON_RESPONSE_FLAG_GET_IPC_LPR_NIGHT_MODE(MessageReceive *message);
  void ON_RESPONSE_FLAG_GET_IPCIMAGE_DISPLAY(MessageReceive *message);

  void drawEnhanceArea();
  void initWideDynamicRange(int index);
  void saveEnhancement(int channel);
  void copyEnhancement();
  void showBlcWdrHlcMode();
  void initExposureTime();
  void initHighLightCompensation(int type);
  void getStructMulti(req_image_enhancement *req);

private slots:
  void onLanguageChanged();

  void on_comboBox_WhiteBalance_activated(int index);
  void on_comboBox_DigitalAntifogMode_activated(int index);

  void on_comboBox_ExposureMode_activated(int index);

  void on_comboBox_BlcRegion_activated(int index);
  void on_comboBox_BlcWdrHlc_activated(int index);
  void on_comboBox_WideDynamicRange_activated(int index);
  void on_comboBox_HighLightCompensation_activated();
  void on_comboBox_reduceBlur_activated();

  void on_pushButton_copy_clicked();
  void on_pushButton_apply_clicked();
  void on_pushButton_back_clicked();
  void onChangeDefog();
  void changeRedGain();
  void changeBlueGain();
  void changeHlcLevel();

private:
  Ui::ImagePageEnhancementMulti *ui;

  MsCameraModel m_model;
  int m_channel;
  IMAGE_ENHANCEMENT_MULTI_S m_info;
  QList<int> m_copyChannelList;
  DrawEnhancement *m_drawEnhanceEx = nullptr;
  PowerLineFrequency m_powerLineFrequency = Frequency_Unknow;
  int m_mainFrameRate = 0;
  bool wdrIsOn = false;
};

#endif // IMAGEPAGEENHANCEMENTMULTI_H

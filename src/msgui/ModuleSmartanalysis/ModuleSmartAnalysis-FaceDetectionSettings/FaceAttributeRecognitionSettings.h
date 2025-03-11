#ifndef FACEATTRIBUTERECOGNITIONSETTINGS_H
#define FACEATTRIBUTERECOGNITIONSETTINGS_H

#include "BaseShadowDialog.h"
#include "QMap"
#include "QCheckBox"

namespace Ui {
class FaceAttributeRecognitionSettings;
}

class FaceAttributeRecognitionSettings : public BaseShadowDialog
{
    Q_OBJECT

public:
    enum NoteType{
        TypeModeError,
        TypeFacePrivacyEnable,
        TypeNone
    };
    explicit FaceAttributeRecognitionSettings(QWidget *parent = nullptr);
    ~FaceAttributeRecognitionSettings();

    void initializeData(int attributeEnable, int attributeType);
    void setIsEnabled(bool enabled);
    int getAttribute();
    int getAttributeType();
    void setNote(const NoteType &note);
    void clear();

private slots:
    void onLanguageChanged();

    void on_pushButtonOk_clicked();
    void on_pushButtonCancel_clicked();

    void onCheckBoxClicked(bool checked);
    void on_checkBoxAll_clicked(bool checked);

    void on_comboBoxRecognition_currentIndexChanged(int index);

private:
    Ui::FaceAttributeRecognitionSettings *ui;
    QMap<int , QCheckBox *> m_checkBoxMap;
    NoteType m_note = TypeNone;
};

#endif // FACEATTRIBUTERECOGNITIONSETTINGS_H

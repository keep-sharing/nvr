#pragma once

#include "BaseShadowDialog.h"

namespace Ui {
class LicensePlateFormatEditor;
}

class LicensePlateFormatEditor : public BaseShadowDialog
{
    Q_OBJECT

public:
    explicit LicensePlateFormatEditor(QWidget *parent = 0, bool isAddingItem = true, int maxNum = 9);
    ~LicensePlateFormatEditor();
    int characterCount();
    QString format();
    void setCharacterCount(int);
    void setFormat(const QString &);
    bool isAddingItem();
private slots:
    void on_pushButton_cancel_clicked();
    void on_pushButton_ok_clicked();
    void on_comboBox_indexSet(int index);
private:
    bool sanityCheck();
    Ui::LicensePlateFormatEditor *ui;
    bool m_isAddingItem = false;
    int currentCharacterCount = -1;
};


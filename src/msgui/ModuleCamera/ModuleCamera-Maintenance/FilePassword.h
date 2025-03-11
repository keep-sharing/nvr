#ifndef FILEPASSWORD_H
#define FILEPASSWORD_H

#include "BaseShadowDialog.h"

namespace Ui {
class FilePassword;
}

class FilePassword : public BaseShadowDialog
{
    Q_OBJECT

public:
    enum FilePasswordMode {
        ModeEncrption,
        ModeDecryption
    };
    explicit FilePassword(QWidget *parent = nullptr);
    ~FilePassword();
    void setMode(int mode);
    QString getPassword();

private slots:
    void onLanguageChanged();
    void on_pushButtonOK_clicked();
    void on_pushButtonCancel_clicked();
private:
    Ui::FilePassword *ui;
    int m_mode = 0;
};

#endif // FILEPASSWORD_H

#ifndef MESSAGEBOX_H
#define MESSAGEBOX_H

#include "BaseShadowDialog.h"
#include "MsLanguage.h"
#include <QMessageBox>

class QTimer;

namespace Ui {
class MessageBox;
}

class MessageBox : public BaseShadowDialog {
    Q_OBJECT

public:
    enum Result {
        Yes,
        Cancel,
        OK
    };

    enum Alignment {
        AlignNone,
        AlignTop,
        AlignCenter
    };

    explicit MessageBox(QWidget *parent = 0);
    ~MessageBox();

    static void initialize(QWidget *parent);
    static MessageBox *instance();

    void startTimer(int interval);

    static Result information(QWidget *parent, const QString &text, const QString &detail = QString(), MessageBox::Alignment alignment = MessageBox::AlignNone);
    static Result information(QWidget *parent, const QString &text, MessageBox::Alignment alignment);
    static Result question(QWidget *parent, const QString &text, const QString &detail = QString(), MessageBox::Alignment alignment = MessageBox::AlignNone);
    static Result question(QWidget *parent, const QString &text, MessageBox::Alignment alignment);
    static Result question(QWidget *parent, const QString &text, const QString &buttonText1, const QString &buttonText2);
    static Result warning(QWidget *parent, const QString &text, const QString &detail = QString());
    static Result englishQuestion(QWidget *parent, const QString &text, const QString &detail = QString());
    //不显示按钮，一般用于重启
    static void message(QWidget *parent, const QString &text, const QString &detail = QString());

    static void queuedInformation(const QString &text, QWidget *parent = nullptr);

    void setTitle(const QString &text);
    void showInformation(const QString &text, const QString &detail);
    void showQuestion(const QString &text, const QString &detail);
    void showQuestion(const QString &text, const QString &buttonText1, const QString &buttonText2);
    void showWarning(const QString &text, const QString &detail);
    void showEnglishQuestion(const QString &text, const QString &detail);
    void showMessage(const QString &text, const QString &detail);

    static Result timeoutQuestion(QWidget *parent, const QString &title, const QString &text, int seconds = 15);

protected:
    bool isAddToVisibleList() override;
    void escapePressed() override;
    void returnPressed() override;

private:
    void setText(const QString &text);
    void setDetail(const QString &text);

signals:
    void yesButtonClicked();
    void cancelButtonClicked();
    void okButtonClicked();

public slots:
    void execInformation(const QString &text);
    void showInformation(const QString &text);
    void onShowInformation(const QString &text);
    void onShowAlignCenterInformation(QWidget *widget, const QString &text);

private slots:
    void onLanguageChanged();
    void onTimeout();
    void on_pushButton_yes_clicked();
    void on_pushButton_cancel_clicked();
    void on_pushButton_ok_clicked();

private:
    static MessageBox *s_self;

    Ui::MessageBox *ui;

    QTimer *m_timer = nullptr;
    int m_timeout = 10;
};

#endif // MESSAGEBOX_H

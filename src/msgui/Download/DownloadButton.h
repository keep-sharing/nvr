#ifndef DOWNLOADBUTTON_H
#define DOWNLOADBUTTON_H

#include <QWidget>

class QMovie;
class DownloadPanel;
class NormalLabel;

namespace Ui {
class DownloadButton;
}

class DownloadButton : public QWidget {
    Q_OBJECT

public:
    explicit DownloadButton(QWidget *parent = 0);
    ~DownloadButton();

    void startMovie();
    void stopMovie();

    void setErrorVisible(bool visible);

protected:
    void mousePressEvent(QMouseEvent *) override;

private:
    void showList();

private slots:
    void onLanguageChanged();

private:
    Ui::DownloadButton *ui;

    QMovie *m_movie = nullptr;
    DownloadPanel *m_list = nullptr;

    NormalLabel *m_labelError = nullptr;
};

#endif // DOWNLOADBUTTON_H

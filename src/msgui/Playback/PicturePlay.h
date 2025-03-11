#ifndef PICTUREPLAY_H
#define PICTUREPLAY_H

#include <QTimer>
#include <QWidget>

class MessageReceive;

namespace Ui {
class PicturePlay;
}

class PicturePlay : public QWidget {
    Q_OBJECT

public:
    explicit PicturePlay(QWidget *parent = 0);
    ~PicturePlay();

    static PicturePlay *instance();

    void initializeData();
    void showImageInfo(const QString &info);
    void showImage(MessageReceive *message);
    void clear();

protected:
    void paintEvent(QPaintEvent *) override;

private slots:
    void onLanguageChanged();

    void onAutoPlayTimer();

    void onToolButtonPreviousPlayClicked();
    void onToolButtonPostPlayClicked();
    void onToolButtonPreviousClicked();
    void onToolButtonNextClicked();

private:
    Ui::PicturePlay *ui;

    static PicturePlay *s_picturePlay;

    QTimer *m_autoPlayTimer;
    bool m_isReverse = false;

    QImage m_image;
};

#endif // PICTUREPLAY_H

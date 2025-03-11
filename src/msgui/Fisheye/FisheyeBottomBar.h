#ifndef FISHEYEBOTTOMBAR_H
#define FISHEYEBOTTOMBAR_H

#include <QWidget>
#include <QButtonGroup>

namespace Ui {
class FisheyeBottomBar;
}

class FisheyeBottomBar : public QWidget
{
    Q_OBJECT

public:
    explicit FisheyeBottomBar(QWidget *parent = nullptr);
    ~FisheyeBottomBar();

    static FisheyeBottomBar *instance();

    void initializeData();

    void updateFisheyeButtonState();
    void setFishMount(int mode);
    void setFishDisplay(int mode);
    void setFisheyeAutoTrackButtonEnabled(bool enabled);
    void setFisheyeAutoTrackButtonChecked(bool checked);

private:
    void setFisheyeMode(int mount, int display);

private slots:
    void onLanguageChanged();
    void onMountGroupClicked(int id);
    void onDisplayGroupClicked(int id);

    void on_toolButton_fish_track_clicked(bool checked);
    void on_toolButton_close_clicked();

private:
    static FisheyeBottomBar *s_self;
    Ui::FisheyeBottomBar *ui;

    QButtonGroup *m_mountGroup;
    QButtonGroup *m_displayGroup;
};

#endif // FISHEYEBOTTOMBAR_H

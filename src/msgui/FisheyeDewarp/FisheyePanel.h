#ifndef FISHEYEPANEL_H
#define FISHEYEPANEL_H

#include "MsWidget.h"
#include <QButtonGroup>
#include <QMap>

class QToolButton;

struct FisheyeKey {
    int mode = -1; //FisheyeDewarpControl::Mode
    int channel = -1;
    int sid = -1;
    int screen = -1;

    FisheyeKey()
    {
    }
    FisheyeKey(int _mode, int _channel, int _sid, int _screen)
    {
        mode = _mode;
        channel = _channel;
        sid = _sid;
        screen = _screen;
    }
    bool operator<(const FisheyeKey &other) const
    {
        if (mode != other.mode) {
            return mode < other.mode;
        } else if (channel != other.channel) {
            return channel < other.channel;
        } else if (sid != other.sid) {
            return sid < other.sid;
        } else {
            return screen < other.screen;
        }
    }
    bool operator==(const FisheyeKey &other) const
    {
        return mode == other.mode && channel == other.channel && sid == other.sid && screen == other.screen;
    }
    bool operator!=(const FisheyeKey &other) const
    {
        return mode != other.mode || channel != other.channel || sid != other.sid || screen != other.screen;
    }
};
QDebug operator<<(QDebug dbg, const FisheyeKey &f);

struct PanelState {
    int installation = 0;
    int display = 1;
};

namespace Ui {
class FisheyePanel;
}

class FisheyePanel : public MsWidget {
    Q_OBJECT

public:
    explicit FisheyePanel(QWidget *parent = nullptr);
    ~FisheyePanel();

    static int s_vapiWinId;
    //当前鱼眼操作的通道
    static FisheyeKey s_currentFisheyeKey;
    //
    static int s_controlMode;
    static FisheyeKey fisheyeChannel();
    static FisheyeKey fisheyeChannel(int mode);
    static void setFisheyeChannel(FisheyeKey key);
    static void setFisheyeChannel(int mode, FisheyeKey key);
    static bool hasFisheyeChannel();
    static void clearFisheyeChannel();
    static void clearFisheyeChannel(int mode);

    static int fisheyeDisplayMode(FisheyeKey key);

    static PanelState panelState();

    void initializePanel();
    void closeDewarp();
    bool isDewarpEnabled() const;

protected:
    bool eventFilter(QObject *obj, QEvent *evt) override;

private:
    void clearState();
    void clearState(FisheyeKey key);
    void saveInstallationState(int state);
    void saveDisplayState(int state);

    void setInstallMode(int mode);
    void setDisplayMode(int installMode, int &displayMode);

    void resetDisplayLayout(int installMode);
    void setInstallButtonChecked(QAbstractButton *button);
    void clearInstallButtonChecked();
    void setDisplayButtonChecked(QAbstractButton *button);
    void clearDisplayButtonChecked();

    void enterDewarpState();

    void closeOtherScreenFisheye();

signals:
    void dewarpStateChanged(int state);

private slots:
    void onLanguageChanged();

    void onInstallButtonGroupClicked(int id);
    void onDisplayButtonGroupClicked(int id);

    void on_toolButton_enable_clicked(bool checked);

private:
    static QMap<int, FisheyeKey> s_controlChannelMap; //<control mode, fisheye channel>
    static QMap<FisheyeKey, PanelState> s_panelStateMap;

    Ui::FisheyePanel *ui;

    QButtonGroup *m_installButtonGroup = nullptr;
    QButtonGroup *m_displayButtonGroup = nullptr;

    QList<QToolButton *> m_displayButtonList;
};
#endif // FISHEYEPANEL_H

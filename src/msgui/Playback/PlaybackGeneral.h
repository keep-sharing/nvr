#ifndef PLAYBACKGENERAL_H
#define PLAYBACKGENERAL_H

#include "BasePlayback.h"
#include "BaseWidget.h"
#include "CloseCommonBackup.h"
#include "SearchCommonBackup.h"
#include "StartPlayback.h"
#include "StopPlayback.h"
#include <QModelIndex>

namespace Ui {
class PlaybackGeneral;
}

class PlaybackGeneral : public BasePlayback {
    Q_OBJECT

public:
    explicit PlaybackGeneral(QWidget *parent = 0);
    ~PlaybackGeneral();

    void initializeData();
    void closePlayback();

    void setCurrentSelectedChannel(int channel);

    bool hasFocus();
    bool focusPrevious();
    bool focusNext();

    NetworkResult dealNetworkCommond(const QString &commond);

    void dealMessage(MessageReceive *message) override;
    void ON_RESPONSE_FLAG_GET_MONTH_EVENT(MessageReceive *message);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void returnPressed();

private:
    void selectNextChannel();
    void selectPreviousChannel();
    void selectChannel(int channel);

signals:

private slots:
    void onLanguageChanged();

    void onSearchFinished(int channel);
    void onStartFinished(int channel);
    void onStopFinished(int channel);
    void onCloseFinished(int channel);

    void onItemClicked(int row, int column);
    void onTreeViewEnterPressed();
    //
    void onSelectedDateChanged(const QDate &date);
    //
    void on_pushButton_selectMax_clicked();
    void on_pushButton_clearAll_clicked();

private:
    Ui::PlaybackGeneral *ui;

    QMap<int, SearchCommonBackup *> m_searchMap;
    QMap<int, int> m_searchingMap;

    QMap<int, StartPlayback *> m_startMap;
    QMap<int, int> m_startingMap;

    QMap<int, StopPlayback *> m_stopMap;
    QMap<int, int> m_stoppingMap;

    QMap<int, CloseCommonBackup *> m_closeMap;
    QMap<int, int> m_closingMap;
};

#endif // PLAYBACKGENERAL_H

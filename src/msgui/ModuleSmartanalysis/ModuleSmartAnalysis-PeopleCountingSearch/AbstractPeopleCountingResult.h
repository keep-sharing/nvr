#ifndef ABSTRACTPEOPLECOUNTINGRESULT_H
#define ABSTRACTPEOPLECOUNTINGRESULT_H

#include <QWidget>
#include "MsDevice.h"

class PeopleCountingBackup;
class MyButtonGroup;
class QToolButton;

class AbstractPeopleCountingResult : public QWidget {
    Q_OBJECT

public:
    enum ButtonGroup {
        ButtonPolyline,
        ButtonHistogram
    };

    explicit AbstractPeopleCountingResult(QWidget *parent = nullptr);
    ~AbstractPeopleCountingResult() override;

    void backupCamera(const QRect &rc, int mode, const QList<int> &channels, const QMap<int ,QString> &textMap, const int lineMask);
    void backupRegions(const QRect &rc, int mode, const QList<int> &channels);
    void backupGroup(const QRect &rc, int mode, const QList<int> &groups);

    void setSubTitleForBackup(const QString &text);

protected:
    void showEvent(QShowEvent *) override;

    virtual void onPolylineClicked() = 0;
    virtual void onHistogramClicked() = 0;

signals:
    void backupFinished(int index);

protected slots:
    virtual void onLanguageChanged();
    void onButtonGroupClicked(int id);
    void onBackupFinished(int result);

protected:
    QString m_mainTitle;
    QString m_subTitle;
    //Backup All时使用，没有加最后的组名，导出时再加
    QString m_subTitleForBackup;

    MyButtonGroup *m_buttonGroup = nullptr;
    QToolButton *m_toolButtonPolyline = nullptr;
    QToolButton *m_toolButtonHistogram = nullptr;

    //m_channelList 在camera中指搜索type（入侵/离开）在regional中指选择的region,以前的命名问题
    QList<int> m_channelList;

    PeopleCountingBackup *m_backup = nullptr;
};

#endif // ABSTRACTPEOPLECOUNTINGRESULT_H

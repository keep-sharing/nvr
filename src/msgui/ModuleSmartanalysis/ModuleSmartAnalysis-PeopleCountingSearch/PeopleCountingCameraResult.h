#ifndef PEOPLECOUNTINGCAMERARESULT_H
#define PEOPLECOUNTINGCAMERARESULT_H

#include "AbstractPeopleCountingResult.h"
#include <QMap>

class CameraPeopleCountingPolylineScene;
class CameraPeopleCountingHistogramScene;

namespace Ui {
class PeopleCountingCameraResult;
}

class PeopleCountingCameraResult : public AbstractPeopleCountingResult
{
    Q_OBJECT

public:
    explicit PeopleCountingCameraResult(QWidget *parent = 0);
    ~PeopleCountingCameraResult();

    void setMainTitle(const QString &text);
    void setSubTitle(const QString &text);
    void setLineTab(quint64 lineMask);

    void showCameraResult(const QList<int> &cameraList, const QList<int> checkedCameraList, const QMap<int, QString> textMap);
    void backup();
    void backupAll();

protected:
    void onPolylineClicked() override;
    void onHistogramClicked() override;

private slots:
    void onLanguageChanged() override;
    void showLine();
    void showHistogram();
    void showHistogram2();

    void on_radioButtonHistogram_clicked(bool checked);
    void on_radioButtonHistogram2_clicked(bool checked);

    void on_comboBoxChannel_activated(int index);

    void onTabBarClicked(int index);

private:
    Ui::PeopleCountingCameraResult *ui;

    CameraPeopleCountingPolylineScene *m_lineScene = nullptr;
    CameraPeopleCountingHistogramScene *m_histogramScene = nullptr;

    QList<int> m_checkedCameraList;
    QMap<int ,QString> m_textMap;
    int m_lineMask = 0;

};

#endif // PEOPLECOUNTINGCAMERARESULT_H

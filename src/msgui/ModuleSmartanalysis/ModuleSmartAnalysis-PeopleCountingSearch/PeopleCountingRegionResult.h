#ifndef PEOPLECOUNTINGREGIONRESULT_H
#define PEOPLECOUNTINGREGIONRESULT_H

#include "AbstractPeopleCountingResult.h"

class RegionPeopleCountingPolylineScene;
class RegionPeopleCountingHistogramScene;
class QButtonGroup;
class QLabel;
class PeopleCountingBackup;

namespace Ui {
class PeopleCountingRegionResult;
}

class PeopleCountingRegionResult : public AbstractPeopleCountingResult
{
    Q_OBJECT

public:
    explicit PeopleCountingRegionResult(QWidget *parent = nullptr);
    ~PeopleCountingRegionResult();

    void setMainTitle(const QString &text);
    void setSubTitle(const QString &text);

    void showRegionResult(const QList<int> &cameraList, const QList<int> checkedCameraList, const QMap<int, QString> textMap);
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

private:
    Ui::PeopleCountingRegionResult *ui;

    RegionPeopleCountingPolylineScene *m_lineScene = nullptr;
    RegionPeopleCountingHistogramScene *m_histogramScene = nullptr;

    QList<int> m_checkedCameraList;
    QMap<int ,QString> m_textMap;
};

#endif // PEOPLECOUNTINGREGIONRESULT_H

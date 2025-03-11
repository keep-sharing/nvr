#ifndef PEOPLECOUNTINGGROUPRESULT_H
#define PEOPLECOUNTINGGROUPRESULT_H

#include "AbstractPeopleCountingResult.h"

class LineScene;
class HistogramScene;

namespace Ui {
class PeopleCountingGroupResult;
}

class PeopleCountingGroupResult : public AbstractPeopleCountingResult
{
    Q_OBJECT

public:
    explicit PeopleCountingGroupResult(QWidget *parent = 0);
    ~PeopleCountingGroupResult();

    void setMainTitle(const QString &text);
    void setSubTitle(const QString &text);

    void showGroupResult(const QList<int> &groupList);

    void backup();
    void backupAll();

protected:
    void onPolylineClicked() override;
    void onHistogramClicked() override;

private slots:
    void onTabClicked(int index);

    void showLine();
    void showHistogram();
    void showHistogram2();

    void on_radioButtonHistogram_clicked(bool checked);
    void on_radioButtonHistogram2_clicked(bool checked);

private:
    Ui::PeopleCountingGroupResult *ui;

    LineScene *m_lineScene = nullptr;
    HistogramScene *m_histogramScene = nullptr;

    QList<int> m_groupList;
    int m_currentGroup = 0;
};

#endif // PEOPLECOUNTINGGROUPRESULT_H

#ifndef FISHEYELAYOUT_H
#define FISHEYELAYOUT_H

#include <QWidget>
#include "FisheyeBar.h"

namespace Ui {
class FisheyeLayout;
}

class FisheyeLayout : public QWidget
{
    Q_OBJECT

public:
    explicit FisheyeLayout(QWidget *parent = nullptr);
    ~FisheyeLayout();

    static FisheyeLayout *instance();

    void setLayoutMode(int mode);
    int layoutMode();
    QRect selectedRect();
    QRect selectedGlobalRect();

    void setFisheyeBarVisible(bool visible);
    int currentFisheyeStream();

    FisheyeBar *fisheyeBar();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    void drawSelectedRect(QPainter *painter);
    void moveFisheyeBar();

private slots:
    void onFisheyeButtonClicked(int mode);

private:
    static FisheyeLayout *s_fisheyeLayout;
    Ui::FisheyeLayout *ui;

    int m_layoutMode = -1;
    QRect m_selectedRect;
    int m_selectedStream = 0; //0-3

    bool m_showFisheyeBar = false;
    FisheyeBar *m_fisheyeBar = nullptr;
};

#endif // FISHEYELAYOUT_H

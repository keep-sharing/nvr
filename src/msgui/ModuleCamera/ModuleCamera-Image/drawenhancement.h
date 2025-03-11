#ifndef DRAWENHANCEMENT_H
#define DRAWENHANCEMENT_H

#include <QWidget>
#include "drawmask.h"
#include "blcwidget.h"

class DrawEnhancement : public DrawMask
{
    Q_OBJECT
public:
    explicit DrawEnhancement(QWidget *parent = nullptr);

	void addEnhanceArea(int left, int top, int right, int bottom);
    void addCentreArea();
	void getEnhanceArea(image_exposure_area *area);

    void setBlcType(BlcType type);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

    MaskWidget *makeNewWidget() override;

signals:

public slots:

private:
    BlcType m_blcType;
};

#endif // DRAWENHANCEMENT_H

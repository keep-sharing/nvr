#ifndef DRAWROI_H
#define DRAWROI_H

#include <QWidget>
#include "drawmask.h"
#include "roiwidget.h"

class DrawRoi : public DrawMask
{
    Q_OBJECT
public:
    explicit DrawRoi(QWidget *parent = nullptr);

	void addRoiArea(const image_roi_area &area, int index);
	void getRoiArea(image_roi_area *area_array, int count);
	void setMaxCount(int count);
	void setAreaEnable(int id, bool enable);
	int getRoiWidth(const image_roi_area &area);
	int getRoiHeight(const image_roi_area &area);

protected:
    MaskWidget *makeNewWidget() override;

signals:

public slots:
};

#endif // DRAWROI_H

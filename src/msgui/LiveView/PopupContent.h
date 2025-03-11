#ifndef POPUPCONTENT_H
#define POPUPCONTENT_H

#include <QDialog>
#include "BasePopup.h"

class PopupContent : public QDialog
{
    Q_OBJECT
public:
    explicit PopupContent(QWidget *parent = nullptr);

    static PopupContent *instance();

    void setPopupWidget(BasePopup *widget);
    void showPopupWidget();
    void closePopupWidget(BasePopup::CloseType type);

protected:
    void mousePressEvent(QMouseEvent *event) override;

signals:

public slots:

private:
    static PopupContent *s_popupContent;

    BasePopup *m_popupWidget = nullptr;
};

#endif // POPUPCONTENT_H

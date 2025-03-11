#ifndef DRAWVIEW_H
#define DRAWVIEW_H

#include <QGraphicsView>

class DrawView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit DrawView(QWidget *parent = nullptr);

    void setScene(QGraphicsScene *scene);

    void setGeometry(const QRect &rc);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;

signals:

public slots:
};

#endif // DRAWVIEW_H

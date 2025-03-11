#ifndef ANPRITEMWIDGET_H
#define ANPRITEMWIDGET_H

#include <QWidget>
#include <QDateTime>

namespace Ui {
class AnprItemWidget;
}

class AnprItemWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AnprItemWidget(QWidget *parent = nullptr);
    ~AnprItemWidget();

    int index() const;
    void setIndex(int index);
    void setItemInfo(int channel, const QDateTime &dateTime, const QImage &image);
    QImage image() const;

    bool isSelected() const;
    void setSelected(bool selected);

    bool isInfoVisible() const;
    void setInfoVisible(bool visible);

    bool isChecked() const;
    void setChecked(bool checked);

    void setIsFace(bool value);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *) override;

    void mousePressEvent(QMouseEvent *event) override;

signals:
    void itemClicked(int index);
    void itemChecked(int index, bool checked);

private slots:
    void on_checkBox_info_clicked(bool checked);

private:
    Ui::AnprItemWidget *ui;

    int m_index = -1;
    int m_margin = 6;
    bool m_isSelected = false;
    bool m_infoVisible = false;
    QRect m_selectedRect;
    bool m_isFace =false;
};

#endif // ANPRITEMWIDGET_H

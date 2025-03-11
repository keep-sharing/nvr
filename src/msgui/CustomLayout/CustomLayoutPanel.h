#ifndef CUSTOMLAYOUTPANEL_H
#define CUSTOMLAYOUTPANEL_H

#include <QWidget>

class CustomLayoutKey;
class QListWidgetItem;

namespace Ui {
class CustomLayoutPanel;
}

class CustomLayoutPanel : public QWidget
{
    Q_OBJECT

public:
    explicit CustomLayoutPanel(QWidget *parent = nullptr);
    ~CustomLayoutPanel();

    void initializeData(int screen, const CustomLayoutKey &currentKey, const QStringList &names);

protected:
    void paintEvent(QPaintEvent *) override;
    void hideEvent(QHideEvent *) override;

signals:
    void itemClicked(const QString &name);
    void settingClicked();
    void closed();

private slots:
    void on_listWidget_itemClicked(QListWidgetItem *item);
    void on_toolButtonSettings_clicked();

private:
    Ui::CustomLayoutPanel *ui;

    int m_marginLeft = 2;
    int m_marginTop = 12;
    int m_marginRight = 2;
    int m_marginBottom = 2;
};

#endif // CUSTOMLAYOUTPANEL_H

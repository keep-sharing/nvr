#ifndef IRREGULARLAYOUT_H
#define IRREGULARLAYOUT_H

#include <QButtonGroup>
#include <QWidget>

class CustomLayoutKey;
class QListWidgetItem;

namespace Ui {
class IrregularLayout;
}

class IrregularLayout : public QWidget {
    Q_OBJECT

public:
    explicit IrregularLayout(QWidget *parent = nullptr);
    ~IrregularLayout();

    static IrregularLayout *instance();

    void setLayout14Visible(bool visible);
    void setCurrentLayoutButton(const CustomLayoutKey &key);

    void onLanguageChanged();

protected:
    bool eventFilter(QObject *, QEvent *) override;
    void showEvent(QShowEvent *) override;
    void hideEvent(QHideEvent *) override;
    void resizeEvent(QResizeEvent *) override;
    void mousePressEvent(QMouseEvent *) override;

private:
    void adjustWidget();

    void initializeCustomLayoutMenu();
    void showCustomLayoutMenu();
    void hideCustomLayoutMenu();

signals:
    void layoutButtonClicked(int layout);
    void customLayoutClicked(const QString &name);

private slots:
    void on_listWidgetCustom_itemClicked(QListWidgetItem *item);

    void onLayoutButtonGroupClicked(int id);

private:
    static IrregularLayout *self;

    Ui::IrregularLayout *ui;

    QButtonGroup *m_layoutButtonGroup = nullptr;

    bool m_isCustomEnable = true;
};

#endif // IRREGULARLAYOUT_H

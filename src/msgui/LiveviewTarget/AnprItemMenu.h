#ifndef ANPRITEMMENU_H
#define ANPRITEMMENU_H

#include <QMenu>
#include <QAction>
#include <QWidgetAction>
#include <QLabel>

class AnprItemMenu : public QMenu
{
    Q_OBJECT

public:
    explicit AnprItemMenu(QWidget *parent = nullptr);
    ~AnprItemMenu();

    void setPlate(const QString &plate);

signals:
    void addBlacklist(const QString &plate);
    void addWhitelist(const QString &plate);
    void removeFromList(const QString &plate);

private slots:
    void onLanguageChanged();

    void onActionBlackClicked();
    void onActionWhiteClicked();
    void onActionRemoveClicked();

private:
    QLabel *m_labelTitle = nullptr;
    QWidgetAction *m_menuTitle = nullptr;
    QMenu *m_menuList = nullptr;
    QAction *m_actionBlack = nullptr;
    QAction *m_actionWhite = nullptr;
    QAction *m_actionRemove = nullptr;

    QString m_strPlate;
};

#endif // ANPRITEMMENU_H

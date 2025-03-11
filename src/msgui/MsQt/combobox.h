#ifndef COMBOBOX_H
#define COMBOBOX_H

#include <QComboBox>

class ComboBox : public QComboBox
{
    Q_OBJECT

public:
    explicit ComboBox(QWidget *parent = 0);

    void addTranslatableItem(const QString &key, const QString &defaultValue, const QVariant & userData = QVariant());
    void retranslate();

    void setCurrentIndexFromData(const QVariant &data, int role = Qt::UserRole);
    QVariant currentData(int role = Qt::UserRole);
    int currentIntData(int role = Qt::UserRole);

    void setCurrentIndex(int index);

    void beginEdit();
    void endEdit();
    void editCurrentIndexFromData(const QVariant &data, int role = Qt::UserRole);

    void removeItem(int index);
    void removeItemFromData(const QVariant &data, int role = Qt::UserRole);

    void clear();
    void reActivateIndex();
    void reSetIndex();

    void setPermission(int mode, int permission);
	
protected:
    void keyPressEvent(QKeyEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;

signals:
    //setCurrentIndexFromData, setCurrentIndex触发
    //activated触发
    //beginEdit和endEdit之间不会触发
    void indexSet(int index);
    void showNoPermission();

private slots:
    void onActivated(int index);

protected:
    QMap<int, QPair<QString, QString>> m_languageMap;
	
	bool m_isEnterPressed = false;

    //
    bool m_isEditing = false;

    //permission
    bool m_checkPermission = false;
    int m_mode;
    int m_permission;
};

#endif // COMBOBOX_H

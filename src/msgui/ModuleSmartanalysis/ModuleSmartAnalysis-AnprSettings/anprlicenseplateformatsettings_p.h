#pragma once

#include "anprlicenseplateformatsettings.h"
#include <QObject>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QVector>

class AnprLicensePlateFormatSettingsModel;
class AnprLicensePlateFormatSettingsDelegate;
class AnprLicensePlateFormatSettingsPrivate : public QObject {
    Q_OBJECT

public:
    typedef AnprLicensePlateFormatSettingsModel Model;

public:
    AnprLicensePlateFormatSettingsPrivate(QObject *parent);
    Q_INVOKABLE bool isExistingCharacterCount(int);
    void populateModel();
    void fetchFromModel();
    void translateUI();
public slots:
    void onOpenLicensePlateFormatEditor(bool isAddingItem);
    void onLicensePlateFormatEditorAccepted();
    void onModelRowChanged(const QModelIndex &, int, int);

public:
    ms_lpr_wildcards *wildcards;
    AnprLicensePlateFormatSettingsModel *model;
    AnprLicensePlateFormatSettingsDelegate *delegate;
    AnprLicensePlateFormatSettings *p;
};

class AnprLicensePlateFormatSettingsModel : public QStandardItemModel {
    Q_OBJECT
public:
    enum {
        SelectionRole = Qt::UserRole,
        FunctionRole,
        CheckRole,
        DisableRole
    };
    enum Functions {
        Function_AddLicense = 0,
        Function_EditLicense,
        Function_DeleteLicense,
        Function_EnableLicense
    };
    enum Column {
        Col_ID = 0,
        Col_CharacterCount,
        Col_Format,
        Col_Enable,
        Col_Edit,
        Col_Delete
    };
    static const int MaxRow = 10;
    static const int MinRow = 2;

public:
    AnprLicensePlateFormatSettingsModel(QObject *parent);
    void addOrUpdateLine(int id, const QString &count, const QString &format, bool enable, bool editable = false, bool deletable = false);
    int selectedRow();
    void initialize();
    static const QPair<QString, QString> &headerName(int idx);
signals:
    void openLicensePlateFormatEditor(bool isAddingItem);
public slots:
    void onItemClicked(const QModelIndex &index);

private:
    void addLastLine();
    void loadHeaderData();
    void enableLicense(const QModelIndex &index);
    int m_selectedRow = -1;
    static QVector<QPair<QString, QString>> headerNames;
};

class AnprLicensePlateFormatSettingsDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    typedef AnprLicensePlateFormatSettingsModel Model;

public:
    AnprLicensePlateFormatSettingsDelegate(QObject *parent);
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

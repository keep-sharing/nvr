#ifndef CUSTOMLAYOUTDIALOG_H
#define CUSTOMLAYOUTDIALOG_H

#include "BaseShadowDialog.h"
#include "CustomLayoutInfoList.h"

class QUndoStack;
class CustomLayoutScene;

namespace Ui {
class CustomLayoutDialog;
}

class CustomLayoutDialog : public BaseShadowDialog
{
    Q_OBJECT

    enum TableColumn
    {
        ColumnCheck,
        ColumnName,
        ColumnDelete
    };

public:
    explicit CustomLayoutDialog(QWidget *parent = nullptr);
    ~CustomLayoutDialog();

    void initializeData(int screen, const CustomLayoutInfoList &layouts);
    const CustomLayoutInfoList &allLayouts() const;

    void pushInfo(const CustomLayoutInfo &info);
    void editBaseRowColumn(int row, int column);

    void editCustomLayoutInfo(const CustomLayoutInfo &info);

    CustomLayoutScene *scene();

private:
    void showCustomLayoutTable();

    void addNewLayout();
    void removeLayout(int row);
    void cacheCurrentLayout();

    void removeStack(const QString &name);
    void renameStack(const QString &oldName, const QString &newName);

    void setCurrentLayout(const QString &name);
    void setCurrentLayout(int row);
    void setCurrentLayout(int row, const QString &name);

    int rowFromName(const QString &name);

    void updateUndoRedoButtonState();

    //主辅屏共用Customlayout布局，但是不共用channel配置，这里处理以下
    void copyScreenLayout(int src, int dest);

private slots:
    void onLanguageChanged();

    void onCustomLayoutTableClicked(int row, int column);
    void onCustomLayoutTableDoubleClicked(int row, int column);
    void onCustomLayoutTableNameEditingFinished(int row, int column, const QString &oldText, const QString &newText);

    void on_comboBoxBasic_indexSet(int index);

    void on_toolButtonUndo_clicked();
    void on_toolButtonRedo_clicked();

    void on_pushButtonOk_clicked();
    void on_pushButtonCancel_clicked();

signals:
    void saved();

private:
    Ui::CustomLayoutDialog *ui;

    CustomLayoutScene *m_scene = nullptr;

    int m_screen = 0;
    //key: name
    QMap<QString, QUndoStack *> m_undoStackMap;
    QUndoStack *m_currentUndoStack = nullptr;

    //
    CustomLayoutInfoList m_allLayouts;
};

#endif // CUSTOMLAYOUTDIALOG_H

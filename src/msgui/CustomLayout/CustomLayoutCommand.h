#ifndef CUSTOMLAYOUTCOMMAND_H
#define CUSTOMLAYOUTCOMMAND_H

#include <QUndoCommand>
#include <QRectF>
#include <QMap>
#include "CustomLayoutInfo.h"

class CustomLayoutDialog;

class CustomLayoutCommand : public QUndoCommand
{
public:
    CustomLayoutCommand(CustomLayoutDialog *draw, const CustomLayoutInfo &info, QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    CustomLayoutDialog *m_draw = nullptr;

    //for undo
    CustomLayoutInfo m_undoInfo;
    //for redo
    CustomLayoutInfo m_redoInfo;
};

#endif // CUSTOMLAYOUTCOMMAND_H

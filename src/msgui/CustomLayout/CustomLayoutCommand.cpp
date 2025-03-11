#include "CustomLayoutCommand.h"
#include "CustomLayoutDialog.h"
#include "CustomLayoutScene.h"

CustomLayoutCommand::CustomLayoutCommand(CustomLayoutDialog *draw, const CustomLayoutInfo &info, QUndoCommand *parent) :
    QUndoCommand(parent),
    m_draw(draw),
    m_redoInfo(info)
{

}

void CustomLayoutCommand::undo()
{
    //执行undo
    m_draw->editBaseRowColumn(m_undoInfo.baseRow(), m_undoInfo.baseColumn());
    m_draw->editCustomLayoutInfo(m_undoInfo);
}

void CustomLayoutCommand::redo()
{
    //保存undo信息
    m_undoInfo = m_draw->scene()->customLayoutInfo();
    //执行redo
    m_draw->editBaseRowColumn(m_redoInfo.baseRow(), m_redoInfo.baseColumn());
    m_draw->editCustomLayoutInfo(m_redoInfo);
}

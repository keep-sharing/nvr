#include "SearchAbstract.h"

SearchAbstract::SearchAbstract(QObject *parent)
    : MsObject(parent)
{

}

int SearchAbstract::channel() const
{
    return m_channel;
}

bool SearchAbstract::isSearching() const
{
    return m_state == StateSearching;
}

bool SearchAbstract::isSearchFinished() const
{
    return m_state == StateFinished;
}

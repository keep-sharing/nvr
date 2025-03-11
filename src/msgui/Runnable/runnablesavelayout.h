#ifndef RUNNABLESAVELAYOUT_H
#define RUNNABLESAVELAYOUT_H

#include <QRunnable>

class layout_custom;

class RunnableSaveLayout : public QRunnable {
public:
    explicit RunnableSaveLayout(layout_custom *layout);
    ~RunnableSaveLayout() override;

    void run() override;

private:
    layout_custom *m_layout = nullptr;
};

#endif // RUNNABLESAVELAYOUT_H

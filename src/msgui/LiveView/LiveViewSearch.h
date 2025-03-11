#ifndef LIVEVIEWSEARCH_H
#define LIVEVIEWSEARCH_H

#include "BaseShadowDialog.h"

class MessageReceive;

namespace Ui {
class LiveViewSearch;
}

class LiveViewSearch : public BaseShadowDialog {
    Q_OBJECT

public:
    explicit LiveViewSearch(QWidget *parent = nullptr);
    ~LiveViewSearch();

    void initializeData();
    void dealMessage(MessageReceive *message);

private slots:
    void onLanguageChanged();

private:
    Ui::LiveViewSearch *ui;
};

#endif // LIVEVIEWSEARCH_H

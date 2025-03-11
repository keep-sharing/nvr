#ifndef TABCOPYRIGHTNOTICE_H
#define TABCOPYRIGHTNOTICE_H

#include "AbstractSettingTab.h"
#include <QWidget>

namespace Ui {
class TabCopyrightNotice;
}

class TabCopyrightNotice : public AbstractSettingTab
{
    Q_OBJECT

public:
    explicit TabCopyrightNotice(QWidget *parent = nullptr);
    ~TabCopyrightNotice();

private:
    Ui::TabCopyrightNotice *ui;
};

#endif // TABCOPYRIGHTNOTICE_H

#ifndef SCRIPTFUNCTION_H
#define SCRIPTFUNCTION_H

#include "BaseShadowDialog.h"

namespace Ui {
class ScriptFunction;
}

class ScriptFunction : public BaseShadowDialog
{
    Q_OBJECT

public:
    explicit ScriptFunction(QWidget *parent = nullptr);
    ~ScriptFunction();

private:
    Ui::ScriptFunction *ui;
};

#endif // SCRIPTFUNCTION_H

#ifndef LINEEDITRTSP_H
#define LINEEDITRTSP_H

#include "LineEdit.h"
#include "RtspInfo.h"

class LineEditRtsp : public LineEdit {
    Q_OBJECT

public:
    explicit LineEditRtsp(QWidget *parent = nullptr);

    RtspInfo rtspInfo() const;

    void clear();
    bool isEmpty() const;

protected:
    bool check() override;
    QString tipString() override;

signals:
};

#endif // LINEEDITRTSP_H

#ifndef MSCOLOR_H
#define MSCOLOR_H

#include <QColor>
#include <QObject>

#define gMsColor MsColor::instance()

class MsColor : public QObject {
    Q_OBJECT

public:
    struct ColorInfo {
        QColor text;
    };

    explicit MsColor(QObject *parent = nullptr);

    static MsColor &instance();

    ColorInfo settings;

    QColor backgroundBlue() const;

signals:
};

#endif // MSCOLOR_H

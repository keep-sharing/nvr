#ifndef BREAKPADHANDLER_H
#define BREAKPADHANDLER_H

#include <QObject>

class BreakpadHandler : public QObject {
    Q_OBJECT

public:
    explicit BreakpadHandler(QObject *parent = nullptr);
    ~BreakpadHandler() override;

    static BreakpadHandler &instance();

    void setDumpPath(const QString &path, const QString &newPrefix);

    bool writeDump();

private:
};

#endif // BREAKPADHANDLER_H

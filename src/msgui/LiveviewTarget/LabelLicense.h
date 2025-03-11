#ifndef LABELLICENSE_H
#define LABELLICENSE_H

#include "mylabel.h"

class LabelLicense : public MyLabel
{
    Q_OBJECT
public:
    explicit LabelLicense(QWidget *parent = nullptr);

    void setLicenseType(const QString & type);

    void clear();

protected:
    void paintEvent(QPaintEvent *event) override;

signals:

public slots:

private:
    static QImage s_imageRed;
    static QImage s_imageBlue;

    QString m_licenseType;
};

#endif // LABELLICENSE_H

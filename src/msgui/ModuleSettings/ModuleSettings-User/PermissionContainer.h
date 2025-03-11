#ifndef PERMISSIONCONTAINER_H
#define PERMISSIONCONTAINER_H

/******************************************************************
* @brief    用户权限CheckBox容器，主要用来实现边框和背景
* @author   LiuHuanyu
* @date     2021-11-14
******************************************************************/

#include <QWidget>

class PermissionContainer : public QWidget
{
    Q_OBJECT
public:
    explicit PermissionContainer(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *) override;

signals:

};

#endif // PERMISSIONCONTAINER_H

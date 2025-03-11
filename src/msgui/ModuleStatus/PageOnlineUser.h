#ifndef PAGEONLINEUSER_H
#define PAGEONLINEUSER_H

#include "AbstractSettingPage.h"

namespace Ui {
class OnlineUser;
}

class PageOnlineUser : public AbstractSettingPage {
    Q_OBJECT

    enum TableColumn {
        ColumnCheck,
        ColumnIndex,
        ColumnUserName,
        ColumnUserLevel,
        ColumnIP,
        ColumnUserLoginTime,
        ColumnAddToAccessFilter
    };

public:
    explicit PageOnlineUser(QWidget *parent = nullptr);
    ~PageOnlineUser();

    void initializeData() override;
    void processMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_GET_ONLINE_USER(MessageReceive *message);

private slots:
    void onTableClicked(int row, int column);

    void on_pushButton_refresh_clicked();
    void on_pushButton_back_clicked();

private:
    Ui::OnlineUser *ui;
};

#endif // PAGEONLINEUSER_H

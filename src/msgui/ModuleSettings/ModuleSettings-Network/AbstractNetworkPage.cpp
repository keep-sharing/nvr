#include "AbstractNetworkPage.h"
#include <stdio.h>

AbstractNetworkPage::AbstractNetworkPage(QWidget *parent)
    : AbstractSettingTab(parent)
{
    get_mac_addr(mac0, mac1);
}

int AbstractNetworkPage::get_mac_addr(char mac0[], char mac1[])
{
    //bruce.milesight debug notyet
    FILE *fp = fopen("/tmp/mac", "rb");
    if (!fp) {
        snprintf(mac0, 32, "%s", "1C:C3:16:0A:19:C8");
        snprintf(mac1, 32, "%s", "1C:C3:16:0A:19:C9");
        return -1;
    }
    fgets(mac0, 32, fp);
    fgets(mac1, 32, fp);
    fclose(fp);
    mac0[17] = '\0';
    mac1[17] = '\0';
    //end debug

    return 0;
}

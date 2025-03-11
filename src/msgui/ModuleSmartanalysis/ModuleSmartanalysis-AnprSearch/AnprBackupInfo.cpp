#include "AnprBackupInfo.h"
#include "MsLanguage.h"
#include "centralmessage.h"
#include <QDebug>
AnprBackupInfo::AnprBackupInfo()
{
}

AnprBackupInfo::AnprBackupInfo(MessageReceive *message)
{
    resp_search_anpr_backup *backup = (resp_search_anpr_backup *)message->data;
    channel = backup->chnid;
    sid = backup->sid;
    port = backup->port;
    index = backup->index;
    dateTime = QDateTime::fromString(QString(backup->pTime), "yyyy-MM-dd HH:mm:ss");
    plate = QString(backup->plate);
    type = QString(backup->ptype);
    plateColor = plateColorString(backup->plateColor);
    vehicleType = vehicleTypeString(backup->vehicleType);
    vehicleBrand = vehicleBrandString(backup->brand);
    vehicleColor = plateColorString(backup->vehicleColor);
    vehicleSpeed = backup->speed;
    region = QString(backup->region);
    direction = backup->direction;
    roiId = backup->roiId;
}

AnprBackupInfo::~AnprBackupInfo()
{
}

QString AnprBackupInfo::typeString() const
{
    if (type == QString("Black")) {
        return GET_TEXT("ANPR/103039", "Black");
    } else if (type == QString("White")) {
        return GET_TEXT("ANPR/103040", "White");
    } else if (type == QString("Visitor")) {
        return GET_TEXT("ANPR/103094", "Visitor");
    } else if (type == QString("All")) {
        return GET_TEXT("ANPR/103038", "All");
    } else {
        return type;
    }
}

QString AnprBackupInfo::plateColorString(int value)
{
    QString text;
    switch (value) {
    case ANPR_COLOR_BLACK:
        text = GET_TEXT("ANPR/103104", "Black");
        break;
    case ANPR_COLOR_BLUE:
        text = GET_TEXT("ANPR/103105", "Blue");
        break;
    case ANPR_COLOR_CYAN:
        text = GET_TEXT("ANPR/103106", "Cyan");
        break;
    case ANPR_COLOR_GRAY:
        text = GET_TEXT("ANPR/103107", "Gray");
        break;
    case ANPR_COLOR_GREEN:
        text = GET_TEXT("ANPR/103108", "Green");
        break;
    case ANPR_COLOR_RED:
        text = GET_TEXT("ANPR/103109", "Red");
        break;
    case ANPR_COLOR_WHITE:
        text = GET_TEXT("ANPR/103110", "White");
        break;
    case ANPR_COLOR_YELLOW:
        text = GET_TEXT("ANPR/103111", "Yellow");
        break;
    case ANPR_COLOR_VIOLET:
        text = GET_TEXT("ANPR/103116", "Violet");
        break;
    case ANPR_COLOR_ORANGE:
        text = GET_TEXT("ANPR/103117", "Orange");
        break;
    default:
        text = "N/A";
        break;
    }
    return text;
}

QString AnprBackupInfo::vehicleTypeString(int value)
{
    QString text;
    switch (value) {
    case ANPR_VEHICLE_CAR:
        text = GET_TEXT("ANPR/103112", "Car");
        break;
    case ANPR_VEHICLE_MOTORCYCLE:
        text = GET_TEXT("ANPR/169090", "Motorbike");
        break;
    case ANPR_VEHICLE_BUS:
        text = GET_TEXT("ANPR/103114", "Bus");
        break;
    case ANPR_VEHICLE_TRUCK:
        text = GET_TEXT("ANPR/103115", "Truck");
        break;
    case ANPR_VEHICLE_VAN:
        text = GET_TEXT("ANPR/169078", "Van");
        break;
    case ANPR_VEHICLE_SUV:
        text = GET_TEXT("ANPR/169079", "SUV");
        break;
    case ANPR_VEHICLE_FORKLIFT:
        text = GET_TEXT("ANPR/169080", "Forklift");
        break;
    case ANPR_VEHICLE_EXCAVATOR:
        text = GET_TEXT("ANPR/169081", "Excavator");
        break;
    case ANPR_VEHICLE_TOWTRUCK:
        text = GET_TEXT("ANPR/169082", "Tow Truck");
        break;
    case ANPR_VEHICLE_POLICECAR:
        text = GET_TEXT("ANPR/169083", "Policecar");
        break;
    case ANPR_VEHICLE_FIREENGINE:
        text = GET_TEXT("ANPR/169084", "Fire Engine");
        break;
    case ANPR_VEHICLE_AMBULANCE:
        text = GET_TEXT("ANPR/169085", "Ambulance");
        break;
    case ANPR_VEHICLE_BICYCLE:
        text = GET_TEXT("ANPR/169086", "Bicycle");
        break;
    case ANPR_VEHICLE_EBIKE:
        text = GET_TEXT("ANPR/169087", "Ebike");
        break;
    case ANPR_VEHICLE_OTHER:
        text = GET_TEXT("ANPR/169088", "Others");
        break;
    default:
        text = "N/A";
        break;
    }
    return text;
}

QString AnprBackupInfo::vehicleBrandString(int value)
{
    QString text;
    switch (value) {
    case ANPR_BRAND_AUDI:
        text = GET_TEXT("ANPR/169007", "Audi");
        break;
    case ANPR_BRAND_ASTONMARTIN:
        text = GET_TEXT("ANPR/169008", "Aston Martin");
        break;
    case ANPR_BRAND_ALFAROMEO:
        text = GET_TEXT("ANPR/169009", "Alfa Romeo");
        break;
    case ANPR_BRAND_BUICK:
        text = GET_TEXT("ANPR/169010", "Buick");
        break;
    case ANPR_BRAND_MERCEDESBENZ:
        text = GET_TEXT("ANPR/169011", "Mercedes-Benz");
        break;
    case ANPR_BRAND_BMW:
        text = GET_TEXT("ANPR/169012", "BMW");
        break;
    case ANPR_BRAND_HONDA:
        text = GET_TEXT("ANPR/169013", "Honda");
        break;
    case ANPR_BRAND_PEUGEOT:
        text = GET_TEXT("ANPR/169014", "Peugeot");
        break;
    case ANPR_BRAND_PORSCHE:
        text = GET_TEXT("ANPR/169015", "Porsche");
        break;
    case ANPR_BRAND_BENTLEY:
        text = GET_TEXT("ANPR/169016", "Bentley");
        break;
    case ANPR_BRAND_BUGATTI:
        text = GET_TEXT("ANPR/169017", "Bugatti");
        break;
    case ANPR_BRAND_VOLKSWAGEN:
        text = GET_TEXT("ANPR/169018", "Volkswagen");
        break;
    case ANPR_BRAND_DODGE:
        text = GET_TEXT("ANPR/169019", "Dodge");
        break;
    case ANPR_BRAND_DAEWOO:
        text = GET_TEXT("ANPR/169020", "Daewoo");
        break;
    case ANPR_BRAND_DAIHATSU:
        text = GET_TEXT("ANPR/169021", "Daihatsu");
        break;
    case ANPR_BRAND_TOYOTA:
        text = GET_TEXT("ANPR/169022", "Toyota");
        break;
    case ANPR_BRAND_FORD:
        text = GET_TEXT("ANPR/169023", "Ford");
        break;
    case ANPR_BRAND_FERRARI:
        text = GET_TEXT("ANPR/169024", "Ferrari");
        break;
    case ANPR_BRAND_FIAT:
        text = GET_TEXT("ANPR/169025", "Fiat");
        break;
    case ANPR_BRAND_GMC:
        text = GET_TEXT("ANPR/169026", "GMC");
        break;
    case ANPR_BRAND_MITSUOKA:
        text = GET_TEXT("ANPR/169027", "MITSUOKA");
        break;
    case ANPR_BRAND_HAVAL:
        text = GET_TEXT("ANPR/169028", "Haval");
        break;
    case ANPR_BRAND_GEELY:
        text = GET_TEXT("ANPR/169029", "Geely ");
        break;
    case ANPR_BRAND_JEEP:
        text = GET_TEXT("ANPR/169030", "Jeep");
        break;
    case ANPR_BRAND_JAGUAR:
        text = GET_TEXT("ANPR/169031", "Jaguar");
        break;
    case ANPR_BRAND_CADILLAC:
        text = GET_TEXT("ANPR/169032", "Cadillac");
        break;
    case ANPR_BRAND_CHRYSLER:
        text = GET_TEXT("ANPR/169033", "Chrysler");
        break;
    case ANPR_BRAND_LEXUS:
        text = GET_TEXT("ANPR/169034", "Lexus");
        break;
    case ANPR_BRAND_LANDROVER:
        text = GET_TEXT("ANPR/169035", "Land Rover");
        break;
    case ANPR_BRAND_LINCOLN:
        text = GET_TEXT("ANPR/169036", "Lincoln");
        break;
    case ANPR_BRAND_SUZUKI:
        text = GET_TEXT("ANPR/169037", "Suzuki");
        break;
    case ANPR_BRAND_ROLLSROYCE:
        text = GET_TEXT("ANPR/169038", "Rolls-royce");
        break;
    case ANPR_BRAND_LAMBORGHINI:
        text = GET_TEXT("ANPR/169039", "Lamborghini");
        break;
    case ANPR_BRAND_RENAULT:
        text = GET_TEXT("ANPR/169040", "Renault");
        break;
    case ANPR_BRAND_MAZDA:
        text = GET_TEXT("ANPR/169041", "Mazda");
        break;
    case ANPR_BRAND_MINI:
        text = GET_TEXT("ANPR/169042", "MINI");
        break;
    case ANPR_BRAND_MASERATI:
        text = GET_TEXT("ANPR/169043", "Maserati");
        break;
    case ANPR_BRAND_MAYBACH:
        text = GET_TEXT("ANPR/169044", "Maybach");
        break;
    case ANPR_BRAND_ACURA:
        text = GET_TEXT("ANPR/169045", "Acura");
        break;
    case ANPR_BRAND_OPEL:
        text = GET_TEXT("ANPR/169046", "Opel");
        break;
    case ANPR_BRAND_CHERY:
        text = GET_TEXT("ANPR/169047", "Chery");
        break;
    case ANPR_BRAND_KIA:
        text = GET_TEXT("ANPR/169048", "Kia");
        break;
    case ANPR_BRAND_NISSAN:
        text = GET_TEXT("ANPR/169049", "Nissan");
        break;
    case ANPR_BRAND_SKODA:
        text = GET_TEXT("ANPR/169050", "Skoda");
        break;
    case ANPR_BRAND_MITSUBISHI:
        text = GET_TEXT("ANPR/169051", "Mitsubishi");
        break;
    case ANPR_BRAND_SUBARU:
        text = GET_TEXT("ANPR/169052", "Subaru");
        break;
    case ANPR_BRAND_SMART:
        text = GET_TEXT("ANPR/169053", "Smart");
        break;
    case ANPR_BRAND_SSANGYONG:
        text = GET_TEXT("ANPR/169054", "Ssangyong");
        break;
    case ANPR_BRAND_TESLA:
        text = GET_TEXT("ANPR/169055", "Tesla");
        break;
    case ANPR_BRAND_ISUZU:
        text = GET_TEXT("ANPR/169056", "Isuzu");
        break;
    case ANPR_BRAND_CHEVROLET:
        text = GET_TEXT("ANPR/169057", "Chevrolet");
        break;
    case ANPR_BRAND_CITROEN:
        text = GET_TEXT("ANPR/169058", "Citroen");
        break;
    case ANPR_BRAND_HYUNDAI:
        text = GET_TEXT("ANPR/169059", "Hyundai");
        break;
    case ANPR_BRAND_INFINITY:
        text = GET_TEXT("ANPR/169060", "Infinity");
        break;
    case ANPR_BRAND_MERCURY:
        text = GET_TEXT("ANPR/169061", "Mercury");
        break;
    case ANPR_BRAND_SATURN:
        text = GET_TEXT("ANPR/169062", "Saturn");
        break;
    case ANPR_BRAND_SAAB:
        text = GET_TEXT("ANPR/169063", "SAAB");
        break;
    case ANPR_BRAND_LYNKCO:
        text = GET_TEXT("ANPR/169065", "LYNK&CO");
        break;
    case ANPR_BRAND_MORRISGARAGES:
        text = GET_TEXT("ANPR/169066", "MorrisGarages");
        break;
    case ANPR_BRAND_PAGANI:
        text = GET_TEXT("ANPR/169067", "pagani");
        break;
    case ANPR_BRAND_SPYKER:
        text = GET_TEXT("ANPR/169068", "Spyker");
        break;
    case ANPR_BRAND_BYD:
        text = GET_TEXT("ANPR/169069", "BYD");
        break;
    case ANPR_BRAND_MCLAREN:
        text = GET_TEXT("ANPR/169070", "McLaren");
        break;
    case ANPR_BRAND_KOENIGSEGG:
        text = GET_TEXT("ANPR/169071", "Koenigsegg");
        break;
    case ANPR_BRAND_VOLVO:
        text = GET_TEXT("ANPR/169072", "Volvo");
        break;
    case ANPR_BRAND_LANCIA:
        text = GET_TEXT("ANPR/169073", "Lancia");
        break;
    case ANPR_BRAND_SHELBY:
        text = GET_TEXT("ANPR/169074", "Shelby");
        break;
    case ANPR_BRAND_SEAT:
        text = GET_TEXT("ANPR/169075", "Seat");
        break;
    case ANPR_BRAND_CUPRA:
        text = GET_TEXT("ANPR/169076", "CUPRA");
        break;
    case ANPR_BRAND_DACIA:
        text = GET_TEXT("ANPR/169077", "Dacia");
        break;
    case ANPR_BRAND_DS:
        text = GET_TEXT("ANPR/169064", "DS");
        break;
    default:
        text = "N/A";
        break;
    }
    return text;
}

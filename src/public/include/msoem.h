#ifndef _MS_OEM_H_
#define _MS_OEM_H_
//////////////////////////////////////////////////////////////

#define OEM_ROOT_DIR	"/mnt/nand2"
#define OEM_UPLOAD_OPT	"/mnt/nand2/oem_upload"


#define UI_PATH 		"/opt/app/bin"
#define QT_START_LOGO 	"logo.jpg"
#define QT_RCC			"gui.rcc"
#define WEB_RCC			"web.tar.gz"
#define WEB_LANG_PATH	"webfile/json"
#define OEM_DB			"oem.db"

#define QT_RCC_STANDER	"gui.rcc_bk"
#define START_LOGO_STANDER "logo.jpg_bk"

#define QT_START_LOGO_BAK "logo_bak.jpg"
#define QT_RCC_BAK 		  "gui_bak.rcc"
#define WEB_RCC_BAK 	  "web_bak.tar.gz"

#define UP_NEW_OEM_CFG		"/opt/app/bin/upoem.cfg"
#define UP_SPL_OEM_CFG		"/opt/app/bin/special_oem.cfg"

//oem type
enum OEM_TYPE
{	
	OEM_TYPE_STANDARD = 0,
	OEM_TYPE_NORMAL = 1,	
	OEM_TYPE_KAREL = 7,
	OEM_TYPE_RUSSION_NESA = 14,
	OEM_TYPE_PROKIDS = 15,	
	OEM_TYPE_TW_ALPHAFINITY = 18,
	OEM_TYPE_NORBAIN = 20,
	OEM_TYPE_FRS = 25,
	OEM_TYPE_MARCO = 27,
	OEM_TYPE_JAPAN = 67,
	OEM_TYPE_CANON = 90,
	OEM_TYPE_YEASTAR = 100,
};

//////////////////////////////////////////////////////////////
#endif//_MS_OEM_H_

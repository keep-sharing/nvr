#ifndef __STM_FLASH__
#define __STM_FLASH__

#define FLASH_PAGE_SIZE ((uint16_t)0x800)//bank0 page size 2KB
#define FLASH_USER_START_ADDR   ((uint32_t)0x08019000)   /* Start @ of user Flash area */
#define FLASH_USER_END_ADDR     ((uint32_t)0x0801B8000)   /* End @ of user Flash area */
#define FMC_PAGES_PROTECTED (OB_WP_6 | OB_WP_7)

#define POWER_STATE_ADDR       ((uint32_t)FLASH_USER_START_ADDR + 0)


void init_flash(void);
uint32_t flash_write(uint32_t *pData, uint32_t addr, uint32_t size);
uint32_t flash_read(uint32_t addr);
uint32_t flash_erase(uint32_t StartSector, uint32_t len);
void ms_set_power_state(uint32_t data);
uint32_t ms_get_power_state(void);
#endif

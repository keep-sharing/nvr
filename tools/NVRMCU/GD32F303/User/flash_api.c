#include "gd32f30x.h"
#include "flash_api.h"

static uint32_t wp_value = 0xFFFFFFFF, protected_pages = 0x0;
__IO fmc_state_enum fmc_state = FMC_READY;

void  init_flash(void)
{
    fmc_unlock();
    ob_unlock();
    //fmc_flag_clear(FMC_FLAG_END | FMC_FLAG_WPERR | FMC_FLAG_PGAERR | FMC_FLAG_PGERR);
    fmc_flag_clear(FMC_FLAG_BANK0_END | FMC_FLAG_BANK0_WPERR | FMC_FLAG_BANK0_PGERR);
    wp_value = ob_write_protection_get();
    if ((wp_value | (~FMC_PAGES_PROTECTED)) != 0xFFFFFFFF) {
        /* Erase all the option Bytes */
        fmc_state = ob_erase();

        /* Check if there is write protected pages */
        if (protected_pages != 0x0) {
            /* Restore write protected pages */
            fmc_state = ob_write_protection_enable(protected_pages);
        }
        /* Generate System Reset to load the new option byte values */
        NVIC_SystemReset();
    }
}

uint32_t flash_write(uint32_t *pData, uint32_t addr, uint32_t size)
{
    uint32_t pTmp;
    uint32_t tmp = addr;
    if ((wp_value & FMC_PAGES_PROTECTED) != 0x00) {
//      printf("erase addr:%d, size:%d\n", tmp, addr+size);
        while ((tmp < addr + size) && (fmc_state == FMC_READY)) {
            pTmp = *(uint32_t *)pData;
            fmc_state = fmc_word_program(tmp, pTmp);
//          printf("fmc_state:%d\n", fmc_state);
            tmp = tmp + 4;
            pData = pData + 4;
        }
    }
    return 0;

}

uint32_t flash_read(uint32_t addr)
{
	uint32_t value;
	value = *(uint32_t *)(addr);
	return value;
}

uint32_t flash_erase(uint32_t StartSector, uint32_t len)
{
    uint32_t erase_counter = 0;
    if ((wp_value & FMC_PAGES_PROTECTED) != 0x00) {
//      printf("erase addr:%d\n", StartSector);
        /* Clear All pending flags */
        //fmc_flag_clear(FMC_FLAG_END | FMC_FLAG_WPERR | FMC_FLAG_PGAERR | FMC_FLAG_PGERR );
        fmc_flag_clear(FMC_FLAG_BANK0_END | FMC_FLAG_BANK0_WPERR | FMC_FLAG_BANK0_PGERR);
        for (erase_counter = 0; (erase_counter < len) && (fmc_state == FMC_READY); erase_counter++) {
            fmc_state = fmc_page_erase(StartSector + (FLASH_PAGE_SIZE * erase_counter));
        }
    }
    return 0;
}

void ms_set_power_state(uint32_t data)
{
    flash_erase(POWER_STATE_ADDR, 1);
    flash_write(&data, POWER_STATE_ADDR, 1);
}

uint32_t ms_get_power_state(void)
{
    return flash_read(POWER_STATE_ADDR);
}


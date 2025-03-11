#include <common.h>
#include <command.h>
#include <config.h>
#include <asm/io.h>
#include <asm/arch/IOAddress.h>

/*---------------------------------------------------------------
 * definition
 *---------------------------------------------------------------
 */
typedef enum {
    VIN_720x480,
    VIN_1024x768,
    VIN_1280x720,
    VIN_1920x1080,
} lcd_vin_t;

/* combination: MSB 16bits: codec, LSB 16bit: output format */
typedef enum {
    OUTPUT_BT1120_720P,
    OUTPUT_BT1120_1080P,
    OUTPUT_BT1120_1024x768,
    /* The following is the VESA */
    OUTPUT_VESA = 0x1000,
    OUTPUT_VGA_1280x720 = OUTPUT_VESA,
    OUTPUT_VGA_1920x1080,
    OUTPUT_VGA_1024x768,
    /* for both RGB and BT1120, LCD300 use */
    OUTPUT_1280x720 = (OUTPUT_BT1120_720P | OUTPUT_VGA_1280x720),
    OUTPUT_1920x1080 = (OUTPUT_BT1120_1080P | OUTPUT_VGA_1920x1080),
    OUTPUT_1024x768 = (OUTPUT_BT1120_1024x768 | OUTPUT_VGA_1024x768),
} lcd_output_t;

typedef enum {
    INPUT_FMT_YUV422 = 0,
    INPUT_FMT_RGB888,
    INPUT_FMT_RGB565,
    INPUT_FMT_ARGB1555,
} input_fmt_t;

typedef enum {
    LCD_ID = 0,
    LCD0_ID = LCD_ID,
    LCD1_ID,
    LCD2_ID
} lcd_idx_t;

#define LCD_IDX_T   lcd_idx_t
#define VGA_MAX_CLK	200000	/* 200Mhz */

extern int flcd_main(LCD_IDX_T lcd_idx, lcd_vin_t vin, lcd_output_t output, input_fmt_t in_fmt, u32 frame_base);
#include <common.h>
#include <command.h>
#include <config.h>
#include <asm/io.h>
#include <asm/arch/IOAddress.h>

/*---------------------------------------------------------------
 * definition
 *---------------------------------------------------------------
 */

extern void flcd210_main(int cvbs, u32 frame_base);
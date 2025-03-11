/* DDR Training command codes location. Copy from DDR to SRAM to train DDR. */

.section .image,#alloc

.globl g_ddrtrn_training_cmd_start
g_ddrtrn_training_cmd_start:
.incbin "drivers/ddr/vendor/boot_adaptation/uboot/cmd_bin/ddr_cmd.bin"
.globl g_ddrtrn_training_cmd_end
g_ddrtrn_training_cmd_end:

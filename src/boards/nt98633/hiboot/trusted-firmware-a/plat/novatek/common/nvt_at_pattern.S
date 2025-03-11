/* flash auto-tunning pattern NA51102-2055*/

.section .nvt_at_pattern,"a"

.set PATTERN, 0x0F0F0F0F0F0F0F0F
.rept 0x200
.8byte PATTERN
.endr

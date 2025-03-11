set pagination off

set logging file /mnt/nand/gdb.log

set logging on

handle SIGPIPE nostop print

run -qws

shell cat /tmp/firmware >> /mnt/nand/gdb.log

shell date >> /mnt/nand/gdb.log

bt

info thread

c

set logging off

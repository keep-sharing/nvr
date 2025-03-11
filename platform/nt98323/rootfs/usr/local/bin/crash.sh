# When HDAL driver crash, it calls this script to dump CPU status
echo "l" > /proc/sysrq-trigger 

# For user application, append to edit here to dump what you want

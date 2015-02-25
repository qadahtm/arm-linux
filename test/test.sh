./xzltestprog &
sleep 3
cat /proc/`pgrep xzltestprog`/maps
#sleep 3
#echo 0x40000000 > /proc/ece695/vaddr
#sleep 1
sleep 1
cat /proc/`pgrep xzltestprog`/maps
sleep 3
cat /proc/`pgrep xzltestprog`/maps
dmesg | tail
#dmesg 

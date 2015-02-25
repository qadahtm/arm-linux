./xzltestprog &
echo sleeping for 3 secs && sleep 1
#echo 0x40000000 > /proc/ece695/vaddr
cat /proc/`pgrep xzltestprog`/maps
#echo "sleeping for 1 sec" && sleep 1 
#cat /proc/`pgrep xzltestprog`/maps
#dmesg | tail
echo "sleeping for 8 sec" && sleep 8
cat /proc/`pgrep xzltestprog`/maps
#dmesg
echo "sleeping for 8 sec" && sleep 8
cat /proc/`pgrep xzltestprog`/maps

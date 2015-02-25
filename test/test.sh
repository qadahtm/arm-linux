./xzltestprog &
<<<<<<< HEAD
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
=======
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
>>>>>>> 3671507420ef297babacce3050a9550846fe222b

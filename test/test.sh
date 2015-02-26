sleep 1
cat /proc/`pgrep xzltestprog`/maps
sleep 3
cat /proc/`pgrep xzltestprog`/maps
sleep 3
cat /proc/`pgrep xzltestprog`/maps
dmesg | tail

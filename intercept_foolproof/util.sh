# remove module just for demo
-sudo rmmod hook_hide

# create initial snapshot of sys call table in file sys_table
echo "Checker LKM running..."
./test.sh
./util
sudo rmmod interceptor 

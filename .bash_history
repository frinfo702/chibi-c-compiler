ls
cd 9cc
gcc -o tmp tmp.s
./tmp
echo $?
gcc -o tmp tmp.s
ls
ls
chmod a+x test.sh
./test.sh
gcc -o tmp tmp.s
exit

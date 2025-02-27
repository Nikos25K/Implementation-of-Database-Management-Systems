function rm_files() {
    rm data.db
    rm build/hp_main
}

rm_files
# make bf
make hp

echo " "
echo " "
echo " "

# ./build/bf_main
./build/hp_main
# valgrind ./build/hp_main

rm_files
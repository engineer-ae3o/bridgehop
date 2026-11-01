cd bridgehop
cmake -B build -G Ninja
cmake --build build --target cppcheck; cd ..

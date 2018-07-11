mkdir -p build
mkdir -p bin

cd build

cmake -D CMAKE_BUILD_TYPE=Debug CMAKE_INSTALL_PREFIX=. ..

make
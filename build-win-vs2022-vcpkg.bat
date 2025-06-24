:: Build file usefull if Qt6 was installed through vcpkg. Remember to update the DCMAKE_TOOLCHAIN_FILE path.
@echo off
if not exist build mkdir build
cd build
cmake -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE="C:/vcpkg/scripts/buildsystems/vcpkg.cmake" ..
cmake --build . --config Release

#Here is a exemple for generating FMT module .dll for python on MSYS2
cmake CMakeLists.txt -G "Eclipse CDT4 - Unix Makefiles" -D_ECLIPSE_VERSION=4.18 -DCMAKE_BUILD_TYPE=Release -DOSI_DIR="/home/FORBR3/Cbc-2.10.5/" -DMOSEK_DIR="C:/PROGRA~1/Mosek/" -DBOOST_DIR="/mingw64/" -DGDAL_DIR="/mingw64/" -DGEOS_DIR="/mingw64/include/geos" -DPYTHON_DIR="/mingw64/bin" -DPYTHON_INCLUDE_DIR="/mingw64/include/python3.8" -DPYTHON_LIBRARY="/mingw64/lib/libpython3.8.dll.a"
cmake --build . --config Release
cmake --install . --config Release
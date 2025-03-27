cmake_minimum_required(VERSION 3.15)
project(SDL_TEST)

set(CMAKE_CXX_STANDARD 17)

# Đường dẫn compiler
set(CMAKE_CXX_COMPILER "C:/msys64/ucrt64/bin/g++.exe")
set(CMAKE_C_COMPILER "C:/msys64/ucrt64/bin/gcc.exe")

# Tìm và thêm SDL2 vào project
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(SDL2_net REQUIRED)

# Thêm thư mục chứa file .h của SDL2
include_directories(${SDL2_INCLUDE_DIRS})

# Tạo file thực thi
add_executable(SDL_TEST main.cpp)

# Liên kết tất cả các thư viện SDL2
target_link_libraries(SDL_TEST ${SDL2_LIBRARIES} SDL2_image SDL2_ttf SDL2_mixer SDL2_net)

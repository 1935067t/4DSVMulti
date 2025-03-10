# CMakeの最低バージョンを記述
cmake_minimum_required(VERSION 3.3)

# ソリューション名を指定
project( 4DSVProjects LANGUAGES CXX)


#コンパイルオプション指定
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -DNDEBUG -march=native")
# if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
#     set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /O2")
# else()
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -DNDEBUG -march=native")
# endif()
# add_compile_options(-O3 -DNDEBUG -march=native)

# OpenCVのパッケージを探す
find_package( OpenCV REQUIRED )

#ヘッダファイルのパスを指定
include_directories( ${OpenCV_INCLUDE_DIRS} )

# 実行ファイルとソース指定
add_executable( OmniImage src/OmniImage/main.cpp )
add_executable( OmniVideo src/OmniVideo/main.cpp )
add_executable( 4DSV src/4DSV/main.cpp )

#c++20でコンパイル
target_compile_features( OmniImage PUBLIC cxx_std_20)
target_compile_features( OmniVideo PUBLIC cxx_std_20)
target_compile_features( 4DSV PUBLIC cxx_std_20)

#リンク先のライブラリを指定
target_link_libraries(OmniImage ${OpenCV_LIBS} )
target_link_libraries(OmniVideo ${OpenCV_LIBS} )
target_link_libraries(4DSV ${OpenCV_LIBS} )

#message(STATUS "コンパイルオプション: ${CMAKE_CXX_FLAGS}")
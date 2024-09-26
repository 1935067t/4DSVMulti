set(SOURCE_CODE main CACHE NAME "Target object name")

# CMakeの最低バージョンを記述
cmake_minimum_required(VERSION 3.3)

# ソリューション名を指定
project( ${SOURCE_CODE} LANGUAGES CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -DNDEBUG")

# OpenCVのパッケージを探す
find_package( OpenCV REQUIRED )

#ヘッダファイルのパスを指定
include_directories( ${OpenCV_INCLUDE_DIRS} )

# 実行ファイル名とソース指定（ソースと同じ名前の実行ファイルを作ります）
add_executable( ${SOURCE_CODE} ${SOURCE_CODE}.cpp )
target_compile_features(${SOURCE_CODE} PUBLIC cxx_std_20)

#message(STATUS "コンパイルオプション: ${CMAKE_CXX_FLAGS}")

#リンク先のライブラリを指定
target_link_libraries( ${SOURCE_CODE} ${OpenCV_LIBS} )

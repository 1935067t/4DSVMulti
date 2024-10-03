set(SOURCE_CODE main CACHE NAME "Target object name")
set(EXECUTABLE_NAME 4dsvViewer_MultiWindow)
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
add_executable( ${EXECUTABLE_NAME} ${SOURCE_CODE}.cpp )
target_compile_features(${EXECUTABLE_NAME} PUBLIC cxx_std_20)

#message(STATUS "コンパイルオプション: ${CMAKE_CXX_FLAGS}")

#リンク先のライブラリを指定
target_link_libraries( ${EXECUTABLE_NAME} ${OpenCV_LIBS} )

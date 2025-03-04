
#
#4DSV,OmniImage,OmniVideoそれぞれでビルドしたい場合はこのファイルを各ディレクトリにコピーしてください
#
set(SOURCE_CODE main CACHE NAME "Target object name")

#Viewerという名前は実行ファイル名になるので適当に変えてください
set(EXECUTABLE_NAME Viewer)

# CMakeの最低バージョンを記述
cmake_minimum_required(VERSION 3.3)

# ソリューション名を指定
project( ${SOURCE_CODE} LANGUAGES CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -DNDEBUG -march=native")

# OpenCVのパッケージを探す
find_package( OpenCV REQUIRED )

#ヘッダファイルのパスを指定
include_directories( ${OpenCV_INCLUDE_DIRS} )

# 実行ファイル名とソース指定
add_executable( ${EXECUTABLE_NAME} ${SOURCE_CODE}.cpp )
target_compile_features(${EXECUTABLE_NAME} PUBLIC cxx_std_20)

#リンク先のライブラリを指定
target_link_libraries( ${EXECUTABLE_NAME} ${OpenCV_LIBS} )

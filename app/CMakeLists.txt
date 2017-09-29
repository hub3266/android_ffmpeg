# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html
# Sets the minimum version of CMake required to build the native library.
cmake_minimum_required(VERSION 3.4.1)

include_directories(src/main/cpp/include/)

set(DIRROOT ${PROJECT_SOURCE_DIR}/libs/${ANDROID_ABI})

file(GLOB my_source src/main/cpp/*.cpp src/main/cpp/*.c)


add_library( native-lib
             SHARED
             ${my_source})


find_library(log-lib
              log )

add_library( avcodec-56
             SHARED
             IMPORTED )
set_target_properties( avcodec-56
                       PROPERTIES IMPORTED_LOCATION
                       ${DIRROOT}/libavcodec-56.so )

add_library( avdevice-56
             SHARED
             IMPORTED )
set_target_properties( avdevice-56
                       PROPERTIES IMPORTED_LOCATION
                       ${DIRROOT}/libavdevice-56.so )

add_library( avformat-56
             SHARED
             IMPORTED )
set_target_properties( avformat-56
                       PROPERTIES IMPORTED_LOCATION
                       ${DIRROOT}/libavformat-56.so )

add_library( avutil-54
             SHARED
             IMPORTED )
set_target_properties( avutil-54
                       PROPERTIES IMPORTED_LOCATION
                       ${DIRROOT}/libavutil-54.so )

add_library( postproc-53
             SHARED
             IMPORTED )
set_target_properties( postproc-53
                       PROPERTIES IMPORTED_LOCATION
                       ${DIRROOT}/libpostproc-53.so )

add_library( swresample-1
             SHARED
             IMPORTED )
set_target_properties(swresample-1
                       PROPERTIES IMPORTED_LOCATION
                       ${DIRROOT}/libswresample-1.so )

  add_library(swscale-3
              SHARED
              IMPORTED)
  set_target_properties(swscale-3
                        PROPERTIES IMPORTED_LOCATION
                        ${DIRROOT}/libswscale-3.so)
  add_library(avfilter-5
              SHARED
              IMPORTED)
  set_target_properties(avfilter-5
                        PROPERTIES IMPORTED_LOCATION
                        ${DIRROOT}/libavfilter-5.so)
target_link_libraries( native-lib
                       avcodec-56
                       avdevice-56
                       avformat-56
                       avutil-54
                       postproc-53
                       swresample-1
                       swscale-3
                       android
                       OpenSLES
                       ${log-lib} )
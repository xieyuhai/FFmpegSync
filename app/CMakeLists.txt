
cmake_minimum_required(VERSION 3.4.1)


file(GLOB my_source src/main/cpp/*.cpp  src/main/cpp/*.c)


add_library(
             native-lib
             SHARED
             ${my_source}
             #src/main/cpp/ffmpeg.c
             #src/main/cpp/native-lib.cpp
             )


#find_library(
#              log-lib
#              log )

#引入头文件目录
include_directories(libs/include)

#设置变量（根目录）
set(DIR ../../../../libs)

MESSAGE("路径" DIR)

#引入已经存在的so库
add_library(avcodec-56
            SHARED
            IMPORTED
)


set_target_properties(avcodec-56
                        PROPERTIES IMPORTED_LOCATION
                        ${DIR}/armeabi/libavcodec-56.so
        )


#引入已经存在的so库
add_library(avdevice-56
            SHARED
            IMPORTED
)


set_target_properties(avdevice-56
                        PROPERTIES IMPORTED_LOCATION
                        ${DIR}/armeabi/libavdevice-56.so
        )

#引入已经存在的so库
add_library(avfilter-5
            SHARED
            IMPORTED
)


set_target_properties(avfilter-5
                        PROPERTIES IMPORTED_LOCATION
                        ${DIR}/armeabi/libavfilter-5.so
        )

#引入已经存在的so库
add_library(avformat-56
            SHARED
            IMPORTED
)


set_target_properties(avformat-56
                        PROPERTIES IMPORTED_LOCATION
                        ${DIR}/armeabi/libavformat-56.so
        )

#引入已经存在的so库
add_library(avutil-54
            SHARED
            IMPORTED
)


set_target_properties(avutil-54
                        PROPERTIES IMPORTED_LOCATION
                        ${DIR}/armeabi/libavutil-54.so
        )

#引入已经存在的so库
add_library(postproc-53
            SHARED
            IMPORTED
)


set_target_properties(postproc-53
                        PROPERTIES IMPORTED_LOCATION
                        ${DIR}/armeabi/libpostproc-53.so
        )

#引入已经存在的so库
add_library(swresample-1
            SHARED
            IMPORTED
)


set_target_properties(swresample-1
                        PROPERTIES IMPORTED_LOCATION
                        ${DIR}/armeabi/libswresample-1.so
        )

#引入已经存在的so库
add_library(swscale-3
            SHARED
            IMPORTED
)


set_target_properties(swscale-3
                        PROPERTIES IMPORTED_LOCATION
                        ${DIR}/armeabi/libswscale-3.so
        )

target_link_libraries(
                       native-lib


                       avcodec-56
                       avdevice-56
                       avfilter-5
                       avformat-56
                       avutil-54
                       postproc-53
                       swresample-1
                       swscale-3

                        log
                        #window 的实现在libandroid.so中
                        android
                        OpenSLES
                       #${log-lib}
                       )
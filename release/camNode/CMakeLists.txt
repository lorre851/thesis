cmake_minimum_required(VERSION 3.1.0)

PROJECT(THESIS)

# paths
#LINK_DIRECTORIES(src)                       # source
#LINK_DIRECTORIES(/usr/local/include)
LINK_DIRECTORIES(/usr/local/lib)            # OpenCV
LINK_DIRECTORIES(/usr/include)
#LINK_DIRECTORIES(/usr/lib)

# Build settings
SET(CMAKE_BUILD_TYPE Release)
SET(EXECUTABLE_OUTPUT_PATH build/bin)
SET(LIBRARY_OUTPUT_PATH build/lib)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY build)

# Compiler settings
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
#SET(CMAKE_CXX_LINK_FLAGS "-pg")


SET(OpenCV_LIBRARIES opencv_core opencv_highgui opencv_videoio -lopencv_calib3d opencv_imgproc -lopencv_imgcodecs opencv_ml opencv_objdetect opencv_features2d opencv_flann -lzmq)
SET(OPENNI_LIB_DIR /usr/lib)
SET(OPENNI_INCLUD_DIR /usr/include/ni)
SET(OPENNI_PRIME_SENSOR_MODULE_BIN_DIR /usr/bin)
#SET(CMAKE_C_FLAGS_DEBUG "-D_DEBUG")


ADD_EXECUTABLE(main src/main.cpp src/camerascanner.cpp src/camerascanner.h src/streamgrid.cpp src/streamgrid.h src/stereotools.cpp src/stereotools.h src/cvserver.cpp src/cvserver.h src/zeromq_tools.cpp src/zeromq_tools.h src/config.cpp src/config.h)
TARGET_LINK_LIBRARIES(main ${OpenCV_LIBRARIES})
TARGET_LINK_LIBRARIES(main pthread)
TARGET_LINK_LIBRARIES(main libServus.so)




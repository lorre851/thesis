cmake_minimum_required(VERSION 3.5)
project(trainframe_emu)

set(CMAKE_CXX_STANDARD 11)
set(SOURCE_FILES src/main.cpp src/interfaces.cpp src/interfaces.h src/depthpeopledetector.cpp src/depthpeopledetector.h src/zeromq_tools.h src/zeromq_tools.cpp src/cvclient.h src/cvclient.cpp)
set(OpenCV_LIBRARIES opencv_core opencv_highgui opencv_videoio -lopencv_calib3d opencv_imgproc -lopencv_imgcodecs opencv_ml opencv_objdetect opencv_features2d opencv_flann)



add_executable(trainframe_emu ${SOURCE_FILES})

TARGET_LINK_LIBRARIES(trainframe_emu ${OpenCV_LIBRARIES})
TARGET_LINK_LIBRARIES(trainframe_emu -lzmq)
TARGET_LINK_LIBRARIES(trainframe_emu pthread)
TARGET_LINK_LIBRARIES(trainframe_emu libServus.so)

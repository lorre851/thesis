cmake_minimum_required(VERSION 3.1.0)
project(camServer)


LINK_DIRECTORIES(/usr/local/lib)            # OpenCV
LINK_DIRECTORIES(/usr/include)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")


set(SOURCE_FILES src/main.cpp src/config.cpp src/config.h src/zeromq_tools.cpp src/zeromq_tools.h src/cvclient.cpp src/cvclient.h src/streamgrid.cpp src/streamgrid.h src/depthpeopledetector.cpp src/depthpeopledetector.h src/camserver.cpp src/camserver.h src/interfaces.cpp src/interfaces.h)
#set(SOURCE_FILES src/main.cpp)



SET(OpenCV_LIBRARIES opencv_core opencv_highgui opencv_videoio -lopencv_calib3d opencv_imgproc -lopencv_imgcodecs opencv_ml opencv_objdetect opencv_features2d opencv_flann)



add_executable(camServer ${SOURCE_FILES})
TARGET_LINK_LIBRARIES(camServer ${OpenCV_LIBRARIES})
TARGET_LINK_LIBRARIES(camServer -lzmq)
TARGET_LINK_LIBRARIES(camServer pthread)
TARGET_LINK_LIBRARIES(camServer -lcurlpp -lcurl)
TARGET_LINK_LIBRARIES(camServer libServus.so)


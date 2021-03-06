project(ze_cameras)
cmake_minimum_required(VERSION 2.8.3)

find_package(catkin_simple REQUIRED)
find_package(OpenCV REQUIRED)
catkin_simple()

include(ze_setup)


#############
# LIBRARIES #
#############
set(HEADERS
  include/ze/cameras/camera.hpp
  include/ze/cameras/camera_impl.hpp
  include/ze/cameras/camera_models.hpp
  include/ze/cameras/camera_rig.hpp
  include/ze/cameras/camera_utils.hpp
  include/ze/cameras/camera_yaml_serialization.hpp
  )

set(SOURCES
  src/camera.cpp
  src/camera_rig.cpp
  src/camera_utils.cpp
  src/camera_yaml_serialization.cpp
  src/camera_impl.cpp
  )

cs_add_library(${PROJECT_NAME} ${SOURCES} ${HEADERS})


##########
# GTESTS #
##########
catkin_add_gtest(test_camera_impl test/test_camera_impl.cpp)
target_link_libraries(test_camera_impl ${PROJECT_NAME} ${OpenCV_LIBRARIES})

catkin_add_gtest(test_camera_rig test/test_camera_rig.cpp)
target_link_libraries(test_camera_rig ${PROJECT_NAME} ${OpenCV_LIBRARIES})

catkin_add_gtest(test_camera_utils test/test_camera_utils.cpp)
target_link_libraries(test_camera_utils ${PROJECT_NAME} ${OpenCV_LIBRARIES})

##########
# EXPORT #
##########
cs_install()
cs_export()

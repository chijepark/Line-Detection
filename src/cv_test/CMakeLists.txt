cmake_minimum_required(VERSION 2.8.3)
project(cv_test)
find_package(catkin REQUIRED COMPONENTS
  cv_bridge
  image_transport
  roscpp
  sensor_msgs
  std_msgs
  message_generation
)
add_message_files(
   FILES
   MsgControl.msg
)
generate_messages(
   DEPENDENCIES
   std_msgs
)
catkin_package(
   LIBRARIES cv_test
   CATKIN_DEPENDS cv_bridge image_transport roscpp sensor_msgs std_msgs message_generation
   DEPENDS system_lib 
)
include_directories(
	${catkin_INCLUDE_DIRS}
)

add_executable(image_converter src/image_converter.cpp)
add_dependencies(image_converter ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})
target_link_libraries(image_converter ${catkin_LIBRARIES})

add_executable(listener src/listener.cpp)
add_dependencies(listener &{$PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_RXPORTED_TARGETS})
target_link_libraries(listener ${catkin_LIBRARIES})
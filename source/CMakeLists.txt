## Include headers

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/)
include_directories(include)
include_directories(${PROJECT_SOURCE_DIR}/bin/include)
include_directories(${PROJECT_SOURCE_DIR}/source/BME280/include)
link_directories(${PROJECT_SOURCE_DIR}/bin/lib)

## Sources
file(GLOB CONFIGREADER_SRC src/*.cpp)
add_library( ConfigReader STATIC ${CONFIGREADER_SRC})
link_libraries(ConfigReader bme280 jsoncpp pthread)


include_directories(${MariaDBClient_INCLUDE_DIRS})
add_executable(Slowcontrol Slowcontrol.cpp)
#target_link_libraries(Slowcontrol BME280pp ${MariaDBClient_LIBRARIES} pthread mariadbclientpp I2C)
target_link_libraries(Slowcontrol ${MariaDBClient_LIBRARIES} pthread mariadbclientpp)
install(TARGETS Slowcontrol RUNTIME DESTINATION ${PROJECT_SOURCE_DIR}/bin)



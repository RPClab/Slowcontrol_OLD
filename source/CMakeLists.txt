## Include headers

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/)
include_directories(include)
include_directories(${PROJECT_SOURCE_DIR}/bin/include)
include_directories(${PROJECT_SOURCE_DIR}/source/BME280/include)
link_directories(${PROJECT_SOURCE_DIR}/bin/lib)
link_directories("${CMAKE_INSTALL_PREFIX}/lib")

## Sources
file(GLOB CONFIGREADER_SRC src/*.cpp)
add_library( ConfigReader STATIC ${CONFIGREADER_SRC})
link_libraries(ConfigReader bme280 jsoncpp pthread)

include_directories(${MariaDBClient_INCLUDE_DIRS})
add_executable(GasSlowControl GasSlowcontrol.cpp)
#target_link_libraries(Slowcontrol BME280pp ${MariaDBClient_LIBRARIES} pthread mariadbclientpp I2C)
target_link_libraries(GasSlowControl ${MariaDBClient_LIBRARIES} pthread mariadbclientpp)
install(TARGETS GasSlowControl RUNTIME DESTINATION ${PROJECT_SOURCE_DIR}/bin)

add_executable(LabSlowControl LabSlowcontrol.cpp)
#target_link_libraries(Slowcontrol BME280pp ${MariaDBClient_LIBRARIES} pthread mariadbclientpp I2C)
target_link_libraries(LabSlowControl ${MariaDBClient_LIBRARIES} pthread mariadbclientpp)
install(TARGETS LabSlowControl RUNTIME DESTINATION ${PROJECT_SOURCE_DIR}/bin)

set_if_not( serial_repository "https://github.com/RPClab/serial" )
set_if_not( serial_version "master" )
    # ----- serial package -----
    ExternalProject_Add(
                        serial
                        GIT_REPOSITORY ${serial_repository}
                        GIT_TAG ${serial_version}
                        GIT_PROGRESS TRUE
                        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} 
-DCMAKE_CXX_FLAGS="-fPIC"
                        PREFIX ${CMAKE_CURRENT_BINARY_DIR}/serial-prefix
                        SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/serial
                        INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
                        LIST_SEPARATOR %
                        UPDATE_COMMAND ""
                        )




#find_library(serial REQUIRED)
link_libraries(serial)
add_executable(Weight Weight.cpp)
target_link_libraries(Weight ${MariaDBClient_LIBRARIES} pthread mariadbclientpp)
install(TARGETS Weight RUNTIME DESTINATION ${PROJECT_SOURCE_DIR}/bin)


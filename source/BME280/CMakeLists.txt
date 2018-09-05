## Include headers

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/)
include_directories(include)
include_directories(${PROJECT_SOURCE_DIR}/bin/include)

link_directories(${PROJECT_SOURCE_DIR}/bin/lib)

## Sources
file(GLOB BME280pp_SRC src/*.cpp)
file(GLOB BME280pp_incl include/*.h)
link_libraries(bme280)
add_library( BME280pp STATIC ${BME280pp_SRC})

install(FILES ${BME280pp_incl} DESTINATION ${PROJECT_SOURCE_DIR}/bin/include/)
install(TARGETS BME280pp RUNTIME DESTINATION ${PROJECT_SOURCE_DIR}/lib ARCHIVE DESTINATION ${PROJECT_SOURCE_DIR}/lib)



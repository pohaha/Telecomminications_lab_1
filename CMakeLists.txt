CMAKE_MINIMUM_REQUIRED(VERSION 3.14..3.20)
project(Lab_1 C CXX)

add_executable(${PROJECT_NAME} main.cpp)
list(APPEND EXTRA_INCLUDE_DIRECTORIES "/usr/include/python3.9")
list(APPEND EXTRA_INCLUDE_DIRECTORIES "/usr/lib64/python3.9/site-packages/numpy/core/include")

list(APPEND EXTRA_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/includes")

list(APPEND EXTRA_LINKED_SOURCES python3.9)

target_include_directories(${PROJECT_NAME}
                           PUBLIC ${PROJECT_BINARY_DIR}
                           PUBLIC ${EXTRA_INCLUDE_DIRECTORIES}
                          )

target_link_libraries(${PROJECT_NAME}
                    ${EXTRA_LINKED_SOURCES}
)


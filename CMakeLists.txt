cmake_minimum_required(VERSION 3.16)

project(
  Router
  VERSION 0.1.0
  LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


find_package(LibXml2 REQUIRED)

option(ROUTER_BUILD_TESTS "Tests mit CTest/GTest bauen" ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(router_core STATIC
  src/library.cpp
  src/osmnode.cpp
  src/coordinates.cpp
  src/osmway.cpp
  src/graph.cpp
  src/node.cpp
  src/edge.cpp
  src/socket.cpp
)
target_include_directories(router_core
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    ${CMAKE_CURRENT_SOURCE_DIR}/external/unordered_dense
)
target_compile_features(router_core PUBLIC cxx_std_17)

target_link_libraries(router_core
  PUBLIC
    LibXml2::LibXml2
)

add_executable(router_app
  src/main.cpp
)
target_link_libraries(router_app
  PRIVATE
    router_core
)

if(ROUTER_BUILD_TESTS)
  enable_testing()

  file(GLOB TEST_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/*_test.cpp"
  )

  foreach(test_src ${TEST_SOURCES})
    get_filename_component(test_name ${test_src} NAME_WE)

    add_executable(${test_name} ${test_src})

    target_include_directories(${test_name}
      PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/external/unordered_dense
    )

    target_link_libraries(${test_name}
      PRIVATE
        router_core
    )

    target_compile_definitions(${test_name}
      PRIVATE
        PROJECT_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
    )

    add_test(NAME ${test_name} COMMAND ${test_name})
  endforeach()
endif()

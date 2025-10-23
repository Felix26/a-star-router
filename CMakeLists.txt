cmake_minimum_required(VERSION 3.16)

project(
  Router
  VERSION 0.1.0
  LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# pugixml einbinden
add_subdirectory(external/pugixml-1.15)

option(ROUTER_BUILD_TESTS "Tests mit CTest/GTest bauen" ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(router_core STATIC
  src/library.cpp
  src/node.cpp
  src/coordinates.cpp
)
target_include_directories(router_core
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(router_core PUBLIC cxx_std_17)

add_executable(router_app
  src/main.cpp
)
target_link_libraries(router_app
  PRIVATE
    router_core
    pugixml
)

if(ROUTER_BUILD_TESTS)
  enable_testing()
  # Tests können hier später via add_subdirectory(tests) hinzugefügt werden.
endif()

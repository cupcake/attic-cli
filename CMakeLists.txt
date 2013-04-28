cmake_minimum_required (VERSION 2.8)

set (CMAKE_CXX_COMPILER clang++)
set (CMAKE_C_COMPILER clang)

project(attic-cli)

# Pre-build lib attic
file(GLOB libattic_lib "build/vendor/libattic/lib/libattic.a")
message(STATUS " LIBATTIC FOUND AT : ${libattic_lib}")
if(libattic_lib)
else()
execute_process(COMMAND mkdir vendor)
execute_process(COMMAND cp -r ../vendor/libattic vendor)
execute_process(WORKING_DIRECTORY vendor/libattic COMMAND cmake .)
execute_process(WORKING_DIRECTORY vendor/libattic COMMAND make -j4)
file(GLOB libattic_lib "build/vendor/libattic/lib/libattic.a")
endif(libattic_lib)

include_directories(src)
include_directories(build/vendor/libattic/include)

file(GLOB_RECURSE attic_cli_SOURCE "src/*.cpp")

add_library(imp_libattic STATIC IMPORTED)
set_property(TARGET imp_libattic PROPERTY IMPORTED_LOCATION ${libattic_lib})

add_executable(attic-cli ${attic_cli_SOURCE})

target_link_libraries(attic-cli 
                      boost_system
                      boost_timer
                      boost_chrono
                      boost_filesystem
                      boost_thread
                      boost_date_time
                      imp_libattic
                      pthread
                      rt
                      crypto
                      ssl
                      dl)



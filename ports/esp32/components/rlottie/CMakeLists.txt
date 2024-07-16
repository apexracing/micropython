cmake_minimum_required(VERSION 3.5)

message("注册rlottie组件")
idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS "${CMAKE_CURRENT_LIST_DIR}/rlottie/inc"
                    )

set(LOTTIE_MODULE OFF)
set(LOTTIE_THREAD OFF)
set(BUILD_SHARED_LIBS OFF)
option(BUILD_TESTING OFF)

function(install)
endfunction()

function(export)
endfunction()
add_compile_options("-fno-if-conversion") #在-O2编译选项下没这句编译不过
add_subdirectory(rlottie)
target_link_libraries(${COMPONENT_LIB} INTERFACE rlottie)

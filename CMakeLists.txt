

cmake_minimum_required(
    VERSION
        3.20
        FATAL_ERROR
)



project(msm)

find_package(ktutils REQUIRED)
find_package(fmt REQUIRED)





# Complier settings.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(DEFINED MSM_STATIC)
    set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=c++20 -static")
    message("Compiling with static linking.")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=c++20")
    message("Compiling with dynamic linking.")
endif()



include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/script.cmake)
RunScript(
    "generate/__init__.py"
    "${PROJECT_NAME};${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME};${CMAKE_CURRENT_SOURCE_DIR}/src"
)


file(GLOB_RECURSE SRCS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)


include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/
)



add_executable(msm ${SRCS})


target_link_libraries(msm ktutils::ktutils fmt::fmt)

if (APPLE)
    if (DEFINED MSM_CERT_AUTH)
        add_custom_command(TARGET "msm" POST_BUILD COMMAND codesign --force --deep --sign "${MSM_CERT_AUTH}" ${PROJECT_BINARY_DIR}/msm)
    endif()
endif()
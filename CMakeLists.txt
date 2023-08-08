cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

project(fractals C)

set(CMAKE_C_STANDARD 11)

# download the submodules
find_package(Git QUIET)

if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
    # Update submodules as needed
    option(GIT_SUBMODULE "Check submodules during build" ON)

    if(GIT_SUBMODULE)
        message(STATUS "Submodule update")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            RESULT_VARIABLE GIT_SUBMOD_RESULT)

        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init --recursive failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif()
    endif()
endif()

# check the submodules
if(NOT EXISTS "${PROJECT_SOURCE_DIR}/extern/glfw/CMakeLists.txt")
    message(FATAL_ERROR "The glfw submodule was not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
endif()

if(NOT EXISTS "${PROJECT_SOURCE_DIR}/extern/glew/CMakeLists.txt")
    message(FATAL_ERROR "The glew submodule was not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
endif()

# if(NOT EXISTS "${PROJECT_SOURCE_DIR}/extern/glm/CMakeLists.txt")
# message(FATAL_ERROR "The glm submodule was not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
# endif()
add_executable(${PROJECT_NAME}
    src/main.c)

if(NOT TARGET glfw)
    add_subdirectory(extern/glfw)
endif()

add_subdirectory(extern/glew)

# add_subdirectory(extern/glm)
target_include_directories(${PROJECT_NAME}
    PUBLIC ${CMAKE_BINARY_DIR}
    PUBLIC extern/glfw/include
    PUBLIC extern/glew/include

    # PUBLIC extern/glm
)

# target_link_libraries(${PROJECT_NAME} glfw libglew_static glm)
target_link_libraries(${PROJECT_NAME} glfw libglew_static)
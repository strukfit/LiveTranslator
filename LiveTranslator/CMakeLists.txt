cmake_minimum_required(VERSION 3.16)
project(LiveTranslator LANGUAGES CXX)

SET(AUTOMOC ON)
SET(AUTOUIC ON)

# Enable hot reloading for MSVC compilers, if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# Enable vcpkg support
if(DEFINED ENV{VCPKG_HOME})
    include("$ENV{VCPKG_HOME}/scripts/buildsystems/vcpkg.cmake")
else()
    include("$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
endif()

set(VCPKG_INCLUDE_DIR "$ENV{VCPKG_HOME}/installed/${VCPKG_TARGET_TRIPLET}/include")
set(VCPKG_LIBRARY_DIR "$ENV{VCPKG_HOME}/installed/${VCPKG_TARGET_TRIPLET}/lib")
set(VCPKG_BIN_DIR "$ENV{VCPKG_HOME}/installed/${VCPKG_TARGET_TRIPLET}/bin")

file(GLOB_RECURSE PROJECT_SOURCES "src/*.cpp"  "include/*.h")

include(qt.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR}
    COMPONENTS
        Core
        Gui
        Widgets
)
qt_standard_project_setup()

qt_add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

find_package(OpenCV REQUIRED)

if(NOT OpenCV_FOUND)
    message(FATAL_ERROR "OpenCV not found.")
endif()

target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${OpenCV_INCLUDE_DIRS}
)

link_directories(${OpenCV_LIB_DIR})

set_target_properties(${PROJECT_NAME}
    PROPERTIES
        WIN32_EXECUTABLE TRUE
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ${OpenCV_LIBS}
        Qt::Core
        Qt::Gui
        Qt::Widgets
)


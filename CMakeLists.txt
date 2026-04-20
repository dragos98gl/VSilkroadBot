cmake_minimum_required(VERSION 3.16)
project(VSilkroadBot LANGUAGES CXX)

# ============================================================
#  Build config
# ============================================================
add_compile_definitions(NOMINMAX)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# ============================================================
#  Paths
# ============================================================

set(THIRD_PARTY_DIR "${CMAKE_SOURCE_DIR}/third_party")
if(NOT DEFINED QT_ROOT)
    set(QT_ROOT "C:/Qt/6.10.1/msvc2022_64" CACHE PATH "Path to Qt (msvc x64)")
endif()
set(CMAKE_PREFIX_PATH "${QT_ROOT}/lib/cmake")

if(NOT DEFINED CUDA_ROOT)
    set(CUDA_ROOT "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v12.4" CACHE PATH "Path to CUDA Toolkit")
endif()
set(CUDA_TOOLKIT_ROOT_DIR "${CUDA_ROOT}")

# --- OpenCV ---
set(OPENCV_DIR         "${THIRD_PARTY_DIR}/opencv_install")
set(OpenCV_DIR         "${OPENCV_DIR}")
set(OPENCV_LIB_DIR   "${OPENCV_DIR}/x64/vc17/lib")
set(OPENCV_BIN_DIR   "${OPENCV_DIR}/x64/vc17/bin")
message(STATUS "THIRD_PARTY_DIR = ${OPENCV_BIN_DIR}")
 
# --- Interception ---
set(INTERCEPTION_DIR      "${THIRD_PARTY_DIR}/Interception")
set(INTERCEPTION_LIB_PATH "${INTERCEPTION_DIR}/library")
set(INTERCEPTION_BIN_PATH "${INTERCEPTION_DIR}/library/x64")

# --- nlohmann/json ---
set(JSON_INCLUDE_DIR "${THIRD_PARTY_DIR}/json/include")

# ============================================================
#  Packages
# ============================================================
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
find_package(OpenCV 4.10.0 REQUIRED)

# ============================================================
#  Sources
# ============================================================
add_executable(${PROJECT_NAME}
    main.cpp

    src/Bot/Bot.cpp
    src/Bot/BotNavigation.cpp
    src/Bot/MobTimer.cpp
    src/Bot/SettingsManager.cpp
    src/Bot/NearbyPlayerDebouncer.cpp

    src/CustomUiObj/RectImageView.cpp
    src/CustomUiObj/BotNavigationMapView.cpp
    src/CustomUiObj/SettingsWindowRgbPosWidget.cpp
    src/CustomUiObj/ImagePixelPicker.cpp
    src/CustomUiObj/DoubleImageView.cpp

    src/UiForms/MainWindow/TrainWindow.cpp
    src/UiForms/MainWindow/ModifyExistingModelWindow.cpp
    src/UiForms/MainWindow/MainWindow.cpp
    src/UiForms/MainWindow/BotSettingsMainWindow.cpp
    src/UiForms/Settings/BotKeyPointsSettingsWindow.cpp
    src/UiForms/Settings/BotNavigationSettingsWindow.cpp
    src/UiForms/Settings/BotCombatSettingsWindow.cpp
    src/UiForms/Bot/BotUi.cpp
    src/UiForms/PixelPickerDialog.cpp

    include/Bot/Bot.hpp
    include/Bot/MobTimer.hpp
    include/Bot/SettingsManager.hpp
    include/Bot/BotNavigation.hpp
    include/Bot/FrameProcess.hpp
    include/Bot/CharacterMovement.hpp
    include/Bot/NearbyPlayerDebouncer.hpp

    include/CustomUiObj/RectImageView.hpp
    include/CustomUiObj/DoubleImageView.hpp
    include/CustomUiObj/ImagePixelPicker.hpp
    include/CustomUiObj/BotNavigationMapView.hpp
    include/CustomUiObj/SettingsWindowRgbPosWidget.hpp

    include/UiForms/Bot/BotUi.hpp
    include/UiForms/MainWindow/TrainWindow.hpp
    include/UiForms/MainWindow/ModifyExistingModelWindow.hpp
    include/UiForms/PixelPickerDialog.hpp
    include/UiForms/Settings/BotNavigationSettingsWindow.hpp
    include/UiForms/Settings/BotCombatSettingsWindow.hpp
    include/UiForms/Settings/BotKeyPointsSettingsWindow.hpp
    include/UiForms/MainWindow/BotSettingsMainWindow.hpp
    include/UiForms/MainWindow/MainWindow.hpp

    include/IO/HIDController.hpp
    include/IO/WindowManager.hpp

    include/utils.hpp
    include/interfaces.hpp
    include/types.hpp
    include/Consts.hpp
)

# ============================================================
#  Include dirs
# ============================================================
target_include_directories(${PROJECT_NAME} PRIVATE
    ${JSON_INCLUDE_DIR}
    ${INTERCEPTION_LIB_PATH}
    ${OpenCV_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# ============================================================
#  Link
# ============================================================

set(OPENCV_VERSION "4100")
set(OPENCV_MODULES
    videoio video core imgproc highgui imgcodecs dnn
    cudaarithm cudaimgproc cudafilters cudalegacy
    features2d calib3d
)

foreach(mod ${OPENCV_MODULES})
    target_link_libraries(${PROJECT_NAME} PRIVATE
        "${OPENCV_LIB_DIR}/opencv_${mod}${OPENCV_VERSION}.lib"
    )
endforeach()

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    "${INTERCEPTION_BIN_PATH}/interception.lib"
)

# ============================================================
#  Post-build: copy DLLs to exe directory
# ============================================================

# Interception DLLs
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${INTERCEPTION_BIN_PATH}/interception.dll"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
)

# OpenCV DLLs
set(OPENCV_DLLS_TO_COPY "")
foreach(mod ${OPENCV_MODULES})
    list(APPEND OPENCV_DLLS_TO_COPY "${OPENCV_BIN_DIR}/opencv_${mod}${OPENCV_VERSION}.dll")
endforeach()
list(APPEND OPENCV_DLLS_TO_COPY "${OPENCV_BIN_DIR}/opencv_flann${OPENCV_VERSION}.dll")

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${OPENCV_DLLS_TO_COPY}
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
)
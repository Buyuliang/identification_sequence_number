cmake_minimum_required(VERSION 3.10)

project(CameraApp)

# 查找 Qt 库
find_package(Qt5 REQUIRED COMPONENTS Core Widgets)

# 查找 OpenCV 库
find_package(OpenCV REQUIRED)

# 将所有源文件添加到项目中
set(SOURCES
    main.cpp
    MainWindow.cpp
    CameraThread.cpp
    DeviceManager.cpp
    Detection.cpp
    LogManager.cpp
)

# 需要将头文件也加入
set(HEADERS
    MainWindow.h
    CameraThread.h
    DeviceManager.h
    Detection.h
    LogManager.h
)

# 将 Qt 元对象编译文件加到源列表中
qt5_wrap_cpp(MOC_SOURCES MainWindow.h CameraThread.h DeviceManager.h Detection.h LogManager.h)

# 定义最终的可执行文件
add_executable(CameraApp ${SOURCES} ${MOC_SOURCES})

# 链接 Qt 和 OpenCV 库
target_link_libraries(CameraApp Qt5::Core Qt5::Widgets ${OpenCV_LIBS})

# 安装可执行文件到当前目录下的 install 目录
install(TARGETS CameraApp DESTINATION ${CMAKE_BINARY_DIR}/install)
install(FILES ${CMAKE_SOURCE_DIR}/rknn_ppocr_system_demo 
        DESTINATION ${CMAKE_BINARY_DIR}/install
        PERMISSIONS OWNER_EXECUTE OWNER_READ OWNER_WRITE)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/lib DESTINATION ${CMAKE_BINARY_DIR}/install)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/model DESTINATION ${CMAKE_BINARY_DIR}/install)

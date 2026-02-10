cmake_minimum_required(VERSION 3.16)
project(MyMusic VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 查找Qt依赖
find_package(Qt5 REQUIRED COMPONENTS Core Gui Network Widgets OpenGL Multimedia Sql)

#find_package(PkgConfig REQUIRED)
#pkg_check_modules(AVCODEC REQUIRED libavcodec)
#pkg_check_modules(AVFORMAT REQUIRED libavformat)
#pkg_check_modules(SWSCALE REQUIRED libswscale)
#pkg_check_modules(SWRESAMPLE REQUIRED libswresample)
#pkg_check_modules(AVUTIL REQUIRED libavutil)
find_package(FFMPEG REQUIRED)
find_package(OpenAL REQUIRED)
find_package(Tablib REQUIRED)
find_package(GLOG REQUIRED)
#find_package(PkgConfig CONFIG REQUIRED)
#pkg_check_modules(TAGLIB REQUIRED taglib)
#pkg_check_modules(GLOG REQUIRED libglog)
find_package(CryptoPP REQUIRED)

# 包含目录
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${AVCODEC_INCLUDE_DIRS}
    ${AVFORMAT_INCLUDE_DIRS}
    ${SWSCALE_INCLUDE_DIRS}
    ${SWRESAMPLE_INCLUDE_DIRS}
    ${AVUTIL_INCLUDE_DIRS}
    ${OPENAL_INCLUDE_DIR}
    ${TAGLIB_INCLUDE_DIRS}
    ${GLOG_INCLUDE_DIRS}
    ${CRYPTOPP_INCLUDE_DIRS}
)

# 链接目录
link_directories(
    ${AVCODEC_LIBRARY_DIRS}
    ${AVFORMAT_LIBRARY_DIRS}
    ${SWSCALE_LIBRARY_DIRS}
    ${SWRESAMPLE_LIBRARY_DIRS}
    ${AVUTIL_LIBRARY_DIRS}
    ${TAGLIB_LIBRARY_DIRS}
    ${GLOG_LIBRARY_DIRS}
)

# 收集源代码文件
set(SOURCE_FILES
    main.cpp
    AIChatClient.cpp
    AudioEffectDialog.cpp
    AudioProcessor.cpp
    AudioStreamProcessor.cpp
    BottomPlayWidget.cpp
    BubbleFrame.cpp
    ChatInputWidget.cpp
    ChatPage.cpp
    ChatWidget.cpp
    DragDropLabel.cpp
    Equalizer.cpp
    LocalDataManager.cpp
    LogManager.cpp
    MetaTag.cpp
    NavItemWidget.cpp
    NetworkImageLabel.cpp
    PlaylistItemWidget.cpp
    ProgressBar.cpp
    RotationLabel.cpp
    SearchLine.cpp
    SeparatorWidget.cpp
    SidebarWidget.cpp
    SongListPage.cpp
    SysTray.cpp
    TitleWidget.cpp
    ToneControl.cpp
    UploadWidget.cpp
    UserManager.cpp
    VolumeWidget.cpp
    colorblocklabel.cpp
    complexionwidget.cpp
    funclistwidgetitem.cpp
    global.cpp
    httpmanager.cpp
    logindialog.cpp
    mainwindow.cpp
    registerdialog.cpp
    selectlocmusic_dlg.cpp
    tcpmanager.cpp
    timerbutton.cpp
    windowmanager.cpp
    
    Common/Encrypt/Encrypt.cpp
    
    FFPlayer/AudioDecoder.cpp
    FFPlayer/DecoderThreadManager.cpp
    FFPlayer/FFPlayer.cpp
    FFPlayer/VideoDecoder.cpp
    
    Tool/directoryscannertask.cpp
    Tool/filescannerworker.cpp
    Tool/FileInspectorImp.cpp
    
    tableview/elidedlabel.cpp
    tableview/playitemwidget.cpp
    tableview/songdelegate.cpp
    tableview/sortproxymodel.cpp
    tableview/tableheaderview.cpp
    tableview/tableview.cpp
    tableview/tableviewmodel.cpp
)

# 收集头文件
set(HEADER_FILES
    AIChatClient.h
    AudioEffectDialog.h
    AudioProcessor.h
    AudioStreamProcessor.h
    BottomPlayWidget.h
    BubbleFrame.h
    ChatInputWidget.h
    ChatPage.h
    ChatWidget.h
    DragDropLabel.h
    Equalizer.h
    LocalDataManager.h
    LogManager.h
    MetaTag.h
    NavItemWidget.h
    NetworkImageLabel.h
    PlayStatePrivate.h
    PlaylistItemWidget.h
    ProgressBar.h
    RotationLabel.h
    SearchLine.h
    SeparatorWidget.h
    SidebarWidget.h
    Singleton.h
    SongListPage.h
    SysTray.h
    TitleWidget.h
    ToneControl.h
    UploadWidget.h
    UserManager.h
    VolumeWidget.h
    colorblocklabel.h
    complexionwidget.h
    dataInfo.h
    funclistwidgetitem.h
    global.h
    httpmanager.h
    logindialog.h
    mainwindow.h
    registerdialog.h
    selectlocmusic_dlg.h
    tcpmanager.h
    timerbutton.h
    windowmanager.h
    
    Common/Encrypt/Encrypt.h
    Common/blockqueue.h
    
    FFPlayer/AudioDecoder.h
    FFPlayer/DecoderInterface.h
    FFPlayer/DecoderThreadManager.h
    FFPlayer/FFPlayer.h
    FFPlayer/VideoDecoder.h
    
    Tool/FileInfo.h
    Tool/FileInspectorImp.h
    Tool/FileInspectorInterface.h
    Tool/directoryscannertask.h
    Tool/filescannerworker.h
    
    tableview/elidedlabel.h
    tableview/playitemwidget.h
    tableview/songdelegate.h
    tableview/sortproxymodel.h
    tableview/tableheaderview.h
    tableview/tableview.h
    tableview/tableviewmodel.h
)

# 收集UI文件
set(UI_FILES
    BottomPlayWidget.ui
    UploadWidget.ui
    complexionwidget.ui
    logindialog.ui
    mainwindow.ui
    registerdialog.ui
    selectlocmusic_dlg.ui
    VolumeWidget.ui
)

# 收集资源文件
set(QRC_FILES
    source.qrc
    tableview/res.qrc
)

# 创建可执行文件
add_executable(MyMusic
    ${SOURCE_FILES}
    ${HEADER_FILES}
    ${UI_FILES}
    ${QRC_FILES}
)

# 链接库
target_link_libraries(MyMusic
    Qt5::Core
    Qt5::Gui
    Qt5::Network
    Qt5::Widgets
    Qt5::OpenGL
    Qt5::Multimedia
    Qt5::Sql
    
    ${AVCODEC_LIBRARIES}
    ${AVFORMAT_LIBRARIES}
    ${SWSCALE_LIBRARIES}
    ${SWRESAMPLE_LIBRARIES}
    ${AVUTIL_LIBRARIES}
    
    ${OPENAL_LIBRARY}
    ${TAGLIB_LIBRARIES}
    ${GLOG_LIBRARIES}
    CryptoPP
)

# 编译选项
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(MyMusic PRIVATE -Wall -Wextra -g)
else()
    target_compile_options(MyMusic PRIVATE -Wall -Wextra -O2)
endif()

# 安装配置
install(TARGETS MyMusic
    RUNTIME DESTINATION bin
)

# 安装资源文件
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/source
    DESTINATION share/MyMusic
    FILES_MATCHING PATTERN "*.png" PATTERN "*.jpg" PATTERN "*.ttf" PATTERN "*.qml"
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tableview/source
    DESTINATION share/MyMusic/tableview
    FILES_MATCHING PATTERN "*.png" PATTERN "*.jpg" PATTERN "*.ttf"
)

# 安装配置文件
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/config.ini
    DESTINATION share/MyMusic
)

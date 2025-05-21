cmake_minimum_required(VERSION 3.16)

project(sueis VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui Widgets Sql REQUIRED Charts)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        user.cpp
        account.cpp
        transaction.cpp
        import.cpp
        category.cpp
        analysis.cpp
        abandon.cpp
        CategoryDelegate.cpp
        mainwindow.h
        CategoryDelegate.h
        mainwindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(sueis
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        logo.qrc



    )
  set(UI_FILES
      MainWindow.ui
  )

  qt6_wrap_ui(UI_HEADERS ${UI_FILES})

# Define target properties for Android with Qt 6 as:
#    set_property(TARGET sueis APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(sueis SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(sueis
            ${PROJECT_SOURCES}
        )
    endif()
endif()

add_subdirectory(QXlsx/ )

target_link_libraries(sueis PRIVATE
  Qt${QT_VERSION_MAJOR}::Widgets
  Qt${QT_VERSION_MAJOR}::Core
  Qt${QT_VERSION_MAJOR}::Gui
  Qt${QT_VERSION_MAJOR}::Sql
  Qt${QT_VERSION_MAJOR}::Charts
  QXlsx::QXlsx
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.sueis)
endif()
set_target_properties(sueis PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS sueis
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
if (MSVC)
    # 告诉 MSVC：输入／执行字符集都用 UTF-8
    add_compile_options(
        /source-charset:utf-8
        /execution-charset:utf-8
        # 或者简写为 /utf-8 （VS2015+ 注）
    )
endif()

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(sueis)
endif()

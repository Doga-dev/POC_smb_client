cmake_minimum_required(VERSION 3.16)

project(POC_smb_client VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Quick)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Quick)

set(PROJECT_HEADERS
    d_samba_server.h
    d_source.h
)
set(PROJECT_SOURCES
        d_samba_server.cpp
        d_source.cpp
        main.cpp
        qml.qrc
)


# # Spécifier explicitement le chemin vers le répertoire des en-têtes et la bibliothèque de libsmbclient
# set(SMBCLIENT_INCLUDE_DIR "/usr/include/samba-4.0")  # Par exemple, /usr/local/include ou /usr/include/samba-4.0
# set(SMBCLIENT_LIBRARY_DIR "/usr/lib/x86_64-linux")
# set(SMBCLIENT_LIBRARY "libsmbclient.so")  # Par exemple, /usr/local/lib/libsmbclient.so

set(SMBCLIENT_INCLUDE_DIR "/usr/local/samba/include")
set(SMBCLIENT_LIBRARY_DIR "/usr/local/samba/lib")
set(SMBCLIENT_LIBRARY "libsmbclient.so")


# # Rechercher la bibliothèque smbclient
# find_package(PkgConfig REQUIRED)
# pkg_check_modules(SMBCLIENT REQUIRED libsmbclient.so)

# Inclure les répertoires d'en-têtes de libsmbclient
include_directories(${SMBCLIENT_INCLUDE_DIR})

# Ajouter le répertoire des bibliothèques pour l'édition de liens
link_directories(${SMBCLIENT_LIBRARY_DIR})


if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(POC_smb_client
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET POC_smb_client APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(POC_smb_client SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(POC_smb_client
            ${PROJECT_HEADERS}
            ${PROJECT_SOURCES}
            main1.cpp
            main2.cpp
            main3.cpp
            d_smb_manager.h d_smb_manager.cpp
            i_smb_client.h
            d_samba_client.h d_samba_client.cpp
            d_file_info.h d_file_info.cpp
        )
    endif()
endif()

target_link_libraries(POC_smb_client  PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Quick
    ${SMBCLIENT_LIBRARY}
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.POC_smb_client)
endif()
set_target_properties(POC_smb_client PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS POC_smb_client
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_import_qml_plugins(POC_smb_client)
    qt_finalize_executable(POC_smb_client)
endif()

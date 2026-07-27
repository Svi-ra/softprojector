##**************************************************************************
##
##    softProjector - an open source media projection software
##    Copyright (C) 2017  Vladislav Kobzar
##
##    This program is free software: you can redistribute it and/or modify
##    it under the terms of the GNU General Public License as published by
##    the Free Software Foundation version 3 of the License.
##
##    This program is distributed in the hope that it will be useful,
##    but WITHOUT ANY WARRANTY; without even the implied warranty of
##    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##    GNU General Public License for more details.
##
##    You should have received a copy of the GNU General Public License
##    along with this program.  If not, see <http://www.gnu.org/licenses/>.
##
##**************************************************************************

cmake_minimum_required(VERSION 3.21)

project(SoftProjector VERSION 2.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/ui)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS
    Core Gui Widgets Network Sql Qml Quick QuickWidgets
    PrintSupport Multimedia MultimediaWidgets Svg LinguistTools)

qt_standard_project_setup(REQUIRES 6.5)

# ── Sources ──────────────────────────────────────────────────────────────────
# The main workspace is QML; the remaining .ui files are the secondary dialogs
# (settings, manage data, editors) that still run as widgets.
set(SP_SOURCES
    sources/main.cpp
    sources/softprojector.cpp
    sources/workspace.cpp
    sources/lucideprovider.cpp
    sources/thumbprovider.cpp
    sources/editwidget.cpp
    sources/song.cpp
    sources/bible.cpp
    sources/settingsdialog.cpp
    sources/aboutdialog.cpp
    sources/addsongbookdialog.cpp
    sources/highlight.cpp
    sources/managedatadialog.cpp
    sources/managedata.cpp
    sources/helpdialog.cpp
    sources/songcounter.cpp
    sources/bibleinformationdialog.cpp
    sources/settings.cpp
    sources/generalsettingwidget.cpp
    sources/biblesettingwidget.cpp
    sources/songsettingwidget.cpp
    sources/announcementsettingwidget.cpp
    sources/printpreviewdialog.cpp
    sources/controlbutton.cpp
    sources/passivesettingwidget.cpp
    sources/theme.cpp
    sources/slideshow.cpp
    sources/videoinfo.cpp
    sources/spfunctions.cpp
    sources/slideshoweditor.cpp
    sources/editannouncementdialog.cpp
    sources/announcement.cpp
    sources/schedule.cpp
    sources/picturesettingwidget.cpp
    sources/moduledownloaddialog.cpp
    sources/moduleprogressdialog.cpp
    sources/displaysetting.cpp
    sources/projectordisplayscreen.cpp
    sources/imagegenerator.cpp
    sources/spimageprovider.cpp
)

set(SP_HEADERS
    headers/softprojector.hpp
    headers/workspace.hpp
    headers/lucideprovider.hpp
    headers/thumbprovider.hpp
    headers/editwidget.hpp
    headers/song.hpp
    headers/bible.hpp
    headers/settingsdialog.hpp
    headers/aboutdialog.hpp
    headers/addsongbookdialog.hpp
    headers/highlight.hpp
    headers/managedatadialog.hpp
    headers/managedata.hpp
    headers/helpdialog.hpp
    headers/songcounter.hpp
    headers/bibleinformationdialog.hpp
    headers/settings.hpp
    headers/generalsettingwidget.hpp
    headers/biblesettingwidget.hpp
    headers/songsettingwidget.hpp
    headers/announcementsettingwidget.hpp
    headers/printpreviewdialog.hpp
    headers/controlbutton.hpp
    headers/passivesettingwidget.hpp
    headers/theme.hpp
    headers/slideshow.hpp
    headers/videoinfo.hpp
    headers/spfunctions.hpp
    headers/slideshoweditor.hpp
    headers/editannouncementdialog.hpp
    headers/announcement.hpp
    headers/schedule.hpp
    headers/picturesettingwidget.hpp
    headers/moduledownloaddialog.hpp
    headers/moduleprogressdialog.hpp
    headers/displaysetting.hpp
    headers/projectordisplayscreen.hpp
    headers/imagegenerator.hpp
    headers/spimageprovider.hpp
)

set(SP_FORMS
    ui/editwidget.ui
    ui/settingsdialog.ui
    ui/aboutdialog.ui
    ui/addsongbookdialog.ui
    ui/managedatadialog.ui
    ui/helpdialog.ui
    ui/songcounter.ui
    ui/bibleinformationdialog.ui
    ui/generalsettingwidget.ui
    ui/biblesettingwidget.ui
    ui/songsettingwidget.ui
    ui/announcementsettingwidget.ui
    ui/printpreviewdialog.ui
    ui/passivesettingwidget.ui
    ui/slideshoweditor.ui
    ui/editannouncementdialog.ui
    ui/picturesettingwidget.ui
    ui/moduledownloaddialog.ui
    ui/moduleprogressdialog.ui
    ui/projectordisplayscreen.ui
)

qt_add_executable(SoftProjector WIN32
    ${SP_SOURCES}
    ${SP_HEADERS}
    ${SP_FORMS}
    softprojector.qrc
)

if(WIN32)
    target_sources(SoftProjector PRIVATE softprojector.rc)
endif()

# ── QML workspace module ─────────────────────────────────────────────────────
set(SP_QML_SINGLETONS
    qml/Theme.qml
)

set(SP_QML_FILES
    ${SP_QML_SINGLETONS}
        # Components
        qml/Icon.qml
        qml/IconButton.qml
        qml/ToolBtn.qml
        qml/Btn.qml
        qml/Panel.qml
        qml/InputField.qml
        qml/Dropdown.qml
        qml/StyledScrollBar.qml
        qml/ListRow.qml
        qml/SelectList.qml
        qml/Badge.qml
        qml/EmptyState.qml
        qml/Toolbar.qml
        qml/ToolSep.qml
        qml/Toggle.qml
        qml/SliderBar.qml
        qml/ModalDialog.qml
        qml/NavRail.qml
        qml/AppMenuBar.qml
        qml/Transport.qml
        qml/SplitGrip.qml
        qml/Segmented.qml
        qml/Toaster.qml
        # Panels
        qml/SchedulePanel.qml
        qml/LivePanel.qml
        qml/BiblePanel.qml
        qml/SongsPanel.qml
        qml/AnnouncePanel.qml
        qml/PicturesPanel.qml
        qml/MediaPanel.qml
        # Shell
        qml/AppShell.qml
)

set_source_files_properties(${SP_QML_SINGLETONS} PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

# Keep the files in a qml/ subdirectory on disk but flat inside the module, so
# every type is reachable as SoftProjector.Ui/<Name> with no nested qmldir.
foreach(f IN LISTS SP_QML_FILES)
    get_filename_component(_name ${f} NAME)
    set_source_files_properties(${f} PROPERTIES QT_RESOURCE_ALIAS ${_name})
endforeach()

qt_add_qml_module(SoftProjector
    URI SoftProjector.Ui
    VERSION 1.0
    RESOURCE_PREFIX /qt/qml
    QML_FILES ${SP_QML_FILES}
)

# ── Lucide icon set ──────────────────────────────────────────────────────────
# Only the icons the interface actually draws are compiled in.
file(GLOB SP_LUCIDE RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} icons/lucide/*.svg)
qt_add_resources(SoftProjector "lucide"
    PREFIX "/lucide"
    FILES ${SP_LUCIDE}
)

# ── Translations ─────────────────────────────────────────────────────────────
qt_add_translations(SoftProjector
    TS_FILES
        translations/softpro_de.ts
        translations/softpro_ru.ts
        translations/softpro_cs.ts
        translations/softpro_ua.ts
        translations/softpro_hy.ts
    QM_FILES_OUTPUT_VARIABLE SP_QM_FILES
)

target_include_directories(SoftProjector PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/headers
)

target_link_libraries(SoftProjector PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::Sql
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickWidgets
    Qt6::PrintSupport
    Qt6::Multimedia
    Qt6::MultimediaWidgets
    Qt6::Svg
)

set_target_properties(SoftProjector PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    MACOSX_BUNDLE TRUE
)

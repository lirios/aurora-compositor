# SPDX-FileCopyrightText: 2024 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

add_executable(tst_compositor
    mockclient.cpp mockclient.h
    mockkeyboard.cpp mockkeyboard.h
    mockpointer.cpp mockpointer.h
    mockseat.cpp mockseat.h
    mockxdgoutputv1.cpp mockxdgoutputv1.h
    testcompositor.cpp testcompositor.h
    testkeyboardgrabber.cpp testkeyboardgrabber.h
    testseat.cpp testseat.h
    tst_compositor.cpp
)

aurora_generate_wayland_protocol_client_sources(tst_compositor
    FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/3rdparty/protocol/idle-inhibit-unstable-v1.xml"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/3rdparty/protocol/ivi-application.xml"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/3rdparty/protocol/viewporter.xml"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/3rdparty/protocol/wayland.xml"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/3rdparty/protocol/xdg-output-unstable-v1.xml"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/3rdparty/protocol/xdg-shell.xml"
)

target_link_libraries(tst_compositor
    PRIVATE
        Qt6::Core
        Qt6::CorePrivate
        Qt6::Gui
        Qt6::GuiPrivate
        Qt6::Test
        Liri::AuroraCompositor
        Liri::AuroraCompositorPrivate
        Wayland::Client
        Wayland::Server
)

liri_extend_target(tst_compositor CONDITION FEATURE_aurora_xkbcommon
    PUBLIC_LIBRARIES
        XKB::XKB
)

add_test(NAME tst_compositor
         COMMAND tst_compositor)

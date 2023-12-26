# SPDX-FileCopyrightText: 2022-2024 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

add_executable(aurora-test-subsurface
    main.cpp
    shmwindow.cpp shmwindow.h
    qml.qrc
)

set_target_properties(aurora-test-subsurface
    PROPERTIES OUTPUT_NAME subsurface
)

target_link_libraries(aurora-test-subsurface
    PRIVATE
        Qt6::Core
        Qt6::CorePrivate
        Qt6::Gui
        Qt6::GuiPrivate
        Qt6::Qml
        Qt6::Quick
)

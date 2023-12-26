# SPDX-FileCopyrightText: 2022-2024 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

add_executable(aurora-test-qmlclient
    main.cpp
    qml.qrc
)

set_target_properties(aurora-test-qmlclient
    PROPERTIES OUTPUT_NAME qmlclient
)

target_link_libraries(aurora-test-qmlclient
    PRIVATE
        Qt6::CorePrivate
        Qt6::Gui
        Qt6::GuiPrivate
        Qt6::Quick
)

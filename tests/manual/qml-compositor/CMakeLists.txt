# SPDX-FileCopyrightText: 2022-2024 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

add_executable(aurora-test-qml-compositor
    main.cpp
    qml.qrc
)

set_target_properties(aurora-test-qml-compositor
    PROPERTIES OUTPUT_NAME qml-compositor
)

target_link_libraries(aurora-test-qml-compositor
    PRIVATE
        Qt6::Core
        Qt6::CorePrivate
        Qt6::Gui
        Qt6::GuiPrivate
	Qt6::Qml
	Qt6::Quick
)

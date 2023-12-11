# SPDX-FileCopyrightText: 2022-2024 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

add_executable(aurora-test-scaling-compositor
    main.cpp
    qml.qrc
)

set_target_properties(aurora-test-scaling-compositor
    PROPERTIES OUTPUT_NAME scaling-compositor
)

target_link_libraries(aurora-test-scaling-compositor
    PRIVATE
        Qt6::Gui
	Qt6::Qml
)

// SPDX-FileCopyrightText: 2024 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-FileCopyrightText: 2022 Xaver Hugl <xaver.hugl@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <memory>

namespace Aurora {

namespace Platform {

struct CDeleter
{
    template <typename T>
    void operator()(T *ptr)
    {
        if (ptr)
            free(ptr);
    }
};

template <typename T>
using UniqueCPtr = std::unique_ptr<T, CDeleter>;

} // namespace Platform

} // namespace Aurora

liri_add_module(AuroraGlobalPrivate
    DESCRIPTION
        "Contains configure features shared by Aurora modules"
    SOURCES
        aurorafeatures.h.in
    STATIC
    NO_PKGCONFIG
)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/aurorafeatures.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/aurorafeatures.h"
    @ONLY
)
set(_global_header_features "#include <LiriAuroraGlobalPrivate/aurorafeatures.h>")
liri_extend_target(AuroraGlobalPrivate
    SOURCES
        "${CMAKE_CURRENT_BINARY_DIR}/aurorafeatures.h"
    GLOBAL_HEADER_CONTENT
        "${_global_header_features}"
)

liri_finalize_module(AuroraGlobalPrivate)

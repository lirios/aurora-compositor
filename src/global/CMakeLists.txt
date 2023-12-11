configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/aurorafeatures.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/aurorafeatures.h"
    @ONLY
)

liri_add_module(AuroraGlobalPrivate
    DESCRIPTION
        "Contains configure features shared by Aurora modules"
    SOURCES
        aurorafeatures.h.in
        "${CMAKE_CURRENT_BINARY_DIR}/aurorafeatures.h"
    GLOBAL_HEADER_CONTENT
        "#include <LiriAuroraGlobalPrivate/aurorafeatures.h>"
    STATIC
    NO_PKGCONFIG
)

liri_finalize_module(AuroraGlobalPrivate)

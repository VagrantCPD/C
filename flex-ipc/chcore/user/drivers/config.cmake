
#option(CHCORE_CONFIG_UFS "UFS Driver" ON)

CMAKE_DEPENDENT_OPTION(
    CHCORE_CONFIG_UFS "Enable UFS Driver" ON
    "CHCORE_ARCH STREQUAL \"aarch64\"" OFF)

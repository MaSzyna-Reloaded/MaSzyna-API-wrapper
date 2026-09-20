# Stamps the build with the moment it was made. Run as a custom target on every build, so each
# library and the export made from it carry the same number, and the app can show which one it is.
STRING(TIMESTAMP BUILD_NUMBER "%Y%m%d%H%M%S")
FILE(WRITE "${OUT}" "${BUILD_NUMBER}")
MESSAGE(STATUS "Build number: ${BUILD_NUMBER}")

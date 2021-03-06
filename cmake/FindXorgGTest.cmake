# Find xorg-gtest,with pkg-config. This sets:
#
# XORG_GTEST_FOUND
# XORG_SERVER_INCLUDE_XORG_GTEST
# XORG_SERVER_GTEST_SRC

# xorg-gtest
pkg_check_modules (XORG_SERVER
		   xorg-gtest>=0.7.0
		   xorg-server
		   x11
		   xi)

if (XORG_SERVER_FOUND)
    pkg_get_variable (_xorg_gtest_prefix xorg-gtest prefix)
    pkg_get_variable (_xorg_gtest_include_dir xorg-gtest includedir)
    pkg_get_variable (_xorg_gtest_source_dir xorg-gtest sourcedir)
    pkg_get_variable (_xorg_gtest_cflags xorg-gtest CPPflags)

    set (XORG_SERVER_GTEST_INCLUDES ${XORG_SERVER_INCLUDE_DIRS})
    set (XORG_SERVER_GTEST_LIBRARY_DIRS ${XORG_SERVER_LIBRARIES})
    set (XORG_SERVER_GTEST_LIBRARIES} ${XORG_SERVER_LIBRARIES})
    set (XORG_SERVER_INCLUDE_XORG_GTEST ${_xorg_gtest_include_dir} CACHE PATH "Path to Xorg GTest Headers")
    set (XORG_SERVER_GTEST_SRC ${_xorg_gtest_source_dir} CACHE PATH "Path to Xorg GTest Sources")
    set (XORG_SERVER_GTEST_CFLAGS ${_xorg_gtest_cflags})
    set (XORG_SERVER_GTEST_ROOT ${CMAKE_SOURCE_DIR}/tests/xorg-gtest CACHE PATH "Path to Xorg GTest CMake sources")
    set (COMPIZ_XORG_SYSTEM_TEST_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/tests/xorg-gtest/include CACHE PATH "Path to Compiz Xorg GTest Headers")
    
    message (STATUS "Found xorg-gtest sources at " ${XORG_SERVER_GTEST_SRC})
    set (XORG_GTEST_FOUND ON)

else (XORG_SERVER_FOUND)

    message (WARNING "Could not found xorg-gtest, can't build xserver tests")
    set (XORG_GTEST_FOUND OFF)

endif (XORG_SERVER_FOUND)

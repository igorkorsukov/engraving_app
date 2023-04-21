
set(MODULE xtz_fonts)

set(MODULE_DIR ${CMAKE_CURRENT_LIST_DIR})

set(MODULE_SRC

    ${CMAKE_CURRENT_LIST_DIR}/fontsmodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/fontsmodule.hpp
    ${CMAKE_CURRENT_LIST_DIR}/fontstypes.hpp
    ${CMAKE_CURRENT_LIST_DIR}/ifontsdatabase.hpp
    ${CMAKE_CURRENT_LIST_DIR}/ifontsengine.hpp

    ${CMAKE_CURRENT_LIST_DIR}/internal/fontsdatabase.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/fontsdatabase.hpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/fontsengine.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/fontsengine.hpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/fontprovider.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/fontprovider.hpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/ifontface.hpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/fontfaceft.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/fontfaceft.hpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/fontfacext.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/fontfacext.hpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/fontfacedu.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/fontfacedu.hpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/fontrendercache.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/fontrendercache.hpp
)

add_subdirectory(${THIRDPARTY_DIR}/freetype ./3rdparty/freetype)
add_subdirectory(${THIRDPARTY_DIR}/harfbuzz ./3rdparty/harfbuzz)
add_subdirectory(${THIRDPARTY_DIR}/msdfgen ./3rdparty/msdfgen)

set(MODULE_INCLUDE
    ${THIRDPARTY_DIR}/freetype/include
    ${THIRDPARTY_DIR}/harfbuzz/harfbuzz/src
    ${THIRDPARTY_DIR}/msdfgen/msdfgen-1.4
    )

set(MODULE_LINK
    harfbuzz
    freetype
    msdfgen
    #xtz_global
)

include(SetupModule)


set(MU_ROOT ${CMAKE_CURRENT_LIST_DIR}/MuseScore)
set(PROJECT_ROOT_DIR ${MU_ROOT})

set(CMAKE_MODULE_PATH
    ${MU_ROOT}/build/cmake
    ${CMAKE_MODULE_PATH}
    )

include(SetupBuildEnvironment)

include_directories(
    ${CMAKE_CURRENT_LIST_DIR}
    ${MU_ROOT}/thirdparty/dtl
)

set(NO_QT_SUPPORT ON)

set(GLOBAL_NO_INTERNAL ON)
add_subdirectory(${MU_ROOT}/src/framework/global global)

set(DRAW_NO_INTERNAL ON)
add_subdirectory(${MU_ROOT}/src/framework/draw draw)

set(ENGRAVING_NO_INTERNAL ON)
set(ENGRAVING_NO_ACCESSIBILITY ON)
add_subdirectory(${MU_ROOT}/src/engraving engraving)

set(THIRDPARTY_DIR ${CMAKE_CURRENT_LIST_DIR}/thirdparty)
add_subdirectory(fonts)

add_library(musescore

    ${CMAKE_CURRENT_LIST_DIR}/musescoremodules.cpp
    ${CMAKE_CURRENT_LIST_DIR}/musescoremodules.h

    # global
    ${CMAKE_CURRENT_LIST_DIR}/filesystem.cpp
    ${CMAKE_CURRENT_LIST_DIR}/filesystem.hpp
    ${CMAKE_CURRENT_LIST_DIR}/resourcesregister.cpp
    ${CMAKE_CURRENT_LIST_DIR}/resourcesregister.h
    ${CMAKE_CURRENT_LIST_DIR}/cryptographichash.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cryptographichash.h

    ${CMAKE_CURRENT_LIST_DIR}/resources/fonts_Bravura.qrc.cpp
    ${CMAKE_CURRENT_LIST_DIR}/resources/fonts_Edwin.qrc.cpp
    ${CMAKE_CURRENT_LIST_DIR}/resources/fonts_Leland.qrc.cpp
    ${CMAKE_CURRENT_LIST_DIR}/resources/fonts_MuseScoreTab.qrc.cpp
    ${CMAKE_CURRENT_LIST_DIR}/resources/notations.qrc.cpp
    ${CMAKE_CURRENT_LIST_DIR}/resources/smufl.qrc.cpp

    # engraving
    ${CMAKE_CURRENT_LIST_DIR}/engravingconfiguration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/engravingconfiguration.hpp
    ${CMAKE_CURRENT_LIST_DIR}/engravingfontsprovider.cpp
    ${CMAKE_CURRENT_LIST_DIR}/engravingfontsprovider.hpp
    ${CMAKE_CURRENT_LIST_DIR}/engravingfont.cpp
    ${CMAKE_CURRENT_LIST_DIR}/engravingfont.hpp
    ${CMAKE_CURRENT_LIST_DIR}/isymbolmetrics.hpp
    ${CMAKE_CURRENT_LIST_DIR}/symbolmetricsfm.cpp
    ${CMAKE_CURRENT_LIST_DIR}/symbolmetricsfm.hpp
)

target_include_directories(musescore PUBLIC
    ${MU_ROOT}/src
    ${MU_ROOT}/src/framework
    ${MU_ROOT}/src/framework/global
    ${MU_ROOT}/src/engraving
)

target_link_libraries(musescore
    global
    draw
    xtz_fonts
    engraving
)

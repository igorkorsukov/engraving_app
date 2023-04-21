set(MODULE freetype)
set(MODULE_DIR ${CMAKE_CURRENT_LIST_DIR})

set(MODULE_SRC
 ${CMAKE_CURRENT_LIST_DIR}/freetype/src/autofit/autofit.c
  ${CMAKE_CURRENT_LIST_DIR}/freetype/src/base/ftbase.c
  ${CMAKE_CURRENT_LIST_DIR}/freetype/src/base/ftbbox.c
  ${CMAKE_CURRENT_LIST_DIR}/freetype/src/base/ftbdf.c
  ${CMAKE_CURRENT_LIST_DIR}/freetype/src/base/ftbitmap.c
  ${CMAKE_CURRENT_LIST_DIR}/freetype/src/base/ftcid.c
  ${CMAKE_CURRENT_LIST_DIR}/freetype/src/base/ftfstype.c
  ${CMAKE_CURRENT_LIST_DIR}/freetype/src/base/ftgasp.c
  ${CMAKE_CURRENT_LIST_DIR}/freetype/src/base/ftglyph.c
  ${CMAKE_CURRENT_LIST_DIR}/freetype/src/base/ftgxval.c
  ${CMAKE_CURRENT_LIST_DIR}/freetype/src/base/ftinit.c
  ${CMAKE_CURRENT_LIST_DIR}/freetype/src/base/ftmm.c
  ${CMAKE_CURRENT_LIST_DIR}/freetype/src/base/ftotval.c
  ${CMAKE_CURRENT_LIST_DIR}/freetype/src/base/ftpatent.c
  ${CMAKE_CURRENT_LIST_DIR}/freetype/src/base/ftpfr.c
  ${CMAKE_CURRENT_LIST_DIR}/freetype/src/base/ftstroke.c
  ${CMAKE_CURRENT_LIST_DIR}/freetype/src/base/ftsynth.c
  ${CMAKE_CURRENT_LIST_DIR}/freetype/src/base/ftsystem.c
  ${CMAKE_CURRENT_LIST_DIR}/freetype/src/base/fttype1.c
  ${CMAKE_CURRENT_LIST_DIR}/freetype/src/base/ftwinfnt.c
  ${CMAKE_CURRENT_LIST_DIR}/freetype/src/bdf/bdf.c
  ${CMAKE_CURRENT_LIST_DIR}/freetype/src/bzip2/ftbzip2.c
  ${CMAKE_CURRENT_LIST_DIR}/freetype/src/cache/ftcache.c
  ${CMAKE_CURRENT_LIST_DIR}/freetype/src/cff/cff.c
  ${CMAKE_CURRENT_LIST_DIR}/freetype/src/cid/type1cid.c
  ${CMAKE_CURRENT_LIST_DIR}/freetype/src/gzip/ftgzip.c
  ${CMAKE_CURRENT_LIST_DIR}/freetype/src/lzw/ftlzw.c
  ${CMAKE_CURRENT_LIST_DIR}/freetype/src/pcf/pcf.c
  ${CMAKE_CURRENT_LIST_DIR}/freetype/src/pfr/pfr.c
  ${CMAKE_CURRENT_LIST_DIR}/freetype/src/psaux/psaux.c
  ${CMAKE_CURRENT_LIST_DIR}/freetype/src/pshinter/pshinter.c
  ${CMAKE_CURRENT_LIST_DIR}/freetype/src/psnames/psnames.c
  ${CMAKE_CURRENT_LIST_DIR}/freetype/src/raster/raster.c
  ${CMAKE_CURRENT_LIST_DIR}/freetype/src/sdf/sdf.c
  ${CMAKE_CURRENT_LIST_DIR}/freetype/src/sfnt/sfnt.c
  ${CMAKE_CURRENT_LIST_DIR}/freetype/src/smooth/smooth.c
  ${CMAKE_CURRENT_LIST_DIR}/freetype/src/svg/svg.c
  ${CMAKE_CURRENT_LIST_DIR}/freetype/src/truetype/truetype.c
  ${CMAKE_CURRENT_LIST_DIR}/freetype/src/type1/type1.c
  ${CMAKE_CURRENT_LIST_DIR}/freetype/src/type42/type42.c
  ${CMAKE_CURRENT_LIST_DIR}/freetype/src/winfonts/winfnt.c
  ${CMAKE_CURRENT_LIST_DIR}/freetype/src/base/ftdebug.c
)

set(MODULE_INCLUDE
    ${CMAKE_CURRENT_LIST_DIR}/freetype/include/freetype
    ${CMAKE_CURRENT_LIST_DIR}/freetype/include
    ${THIRDPARTY_DIR}/harfbuzz/harfbuzz/src
)

set(MODULE_DEF
    -DFT_CONFIG_OPTION_USE_HARFBUZZ
    -DFT2_BUILD_LIBRARY
)

set(MODULE_NOT_LINK_GLOBAL ON)
set(MODULE_PCH_DISABLED ON)
set(MODULE_UNITY_DISABLED ON)
include(SetupModule)

target_include_directories(freetype 
    INTERFACE   ${CMAKE_CURRENT_LIST_DIR}/freetype/include/freetype
                ${CMAKE_CURRENT_LIST_DIR}/freetype/include
)

add_library(mscore_freetype ALIAS freetype)

target_no_warning(${MODULE} -Wimplicit-fallthrough=0)
target_no_warning(${MODULE} -Wno-conversion)
target_no_warning(${MODULE} -Wno-cast-function-type)
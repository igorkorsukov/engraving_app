cmake_minimum_required(VERSION 3.14)

project(engraving_app LANGUAGES CXX C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(
    -DNO_QT_SUPPORT
    -DENGRAVING_NO_INTERACTIVE
    -DENGRAVING_NO_INTERNAL
    -DENGRAVING_NO_QTEXTDOCUMENT
    -DENGRAVING_NO_ACCESSIBILITY

    # Adding dummy definitions for Musescore
    -DMUSESCORE_REVISION="abc123"
    -DMUSESCORE_VERSION="4.0.0"
    -DMUSESCORE_VERSION_LABEL="dev"


    -DSOURCE_PATH="${CMAKE_CURRENT_LIST_DIR}"
)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/musescore)

add_executable(engraving_app
    main.cpp
)

target_link_libraries(engraving_app
    musescore
)

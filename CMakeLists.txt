cmake_minimum_required(VERSION 3.20)
project(ApexReader VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS
    Core Gui Widgets Concurrent PrintSupport)

set(PDFIUM_DIR "${CMAKE_SOURCE_DIR}/third_party")

add_library(pdfium SHARED IMPORTED GLOBAL)
set_target_properties(pdfium PROPERTIES
    IMPORTED_LOCATION "${PDFIUM_DIR}/bin/pdfium.dll"
    IMPORTED_IMPLIB "${PDFIUM_DIR}/lib/pdfium.dll.lib"
    INTERFACE_INCLUDE_DIRECTORIES "${PDFIUM_DIR}/include"
)

set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/PdfView.cpp
    src/AnnotationManager.cpp
    src/PdfOperations.cpp
    )

set(HEADERS
    include/MainWindow.h
    include/PdfView.h
    include/AnnotationManager.h
    include/PdfOperations.h
)

qt_add_resources(QRC_FILES assets/icons.qrc)

# Ye rahi aapki single add_executable line
add_executable(${PROJECT_NAME} WIN32
    ${SOURCES} ${HEADERS} ${QRC_FILES} app.rc)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${PDFIUM_DIR}/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core Qt6::Gui Qt6::Widgets
    Qt6::Concurrent Qt6::PrintSupport
    pdfium
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PDFIUM_DIR}/bin/pdfium.dll"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/pdfium.dll"
    COMMENT "Copying pdfium.dll..."
)

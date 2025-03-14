# Add the test executable
add_executable(mxl-clip-player
    main.cpp
    ../../lib/src/internal/FlowParser.cpp
)

# Specify the include directories for the test executable to find the library headers
target_include_directories(mxl-info PRIVATE ${PROJECT_SOURCE_DIR}/lib/include)

find_package(CLI11 CONFIG REQUIRED)
find_package(stduuid CONFIG REQUIRED)
find_package(minizip-ng CONFIG REQUIRED)

target_link_libraries(mxl-clip-player PRIVATE mxl stduuid CLI11::CLI11 MINIZIP::minizip-ng)

# Add the test executable
add_executable(mxl-info
    main.cpp
)

# Specify the include directories for the test executable to find the library headers
target_include_directories(mxl-info PRIVATE ${PROJECT_SOURCE_DIR}/lib/include)

find_package(CLI11 CONFIG REQUIRED)
find_package(stduuid CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
target_link_libraries(mxl-info PRIVATE mxl stduuid CLI11::CLI11 fmt::fmt)

set_target_properties(mxl-info PROPERTIES
    INSTALL_RPATH "$ORIGIN/../lib"
)

# Install targets
install(TARGETS mxl-info
    RUNTIME DESTINATION bin
)
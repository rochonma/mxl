find_package(Catch2 REQUIRED)
find_package(stduuid CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(picojson REQUIRED)
find_package(PcapPlusPlus CONFIG)

# Add the test executable
add_executable(mxl-tests
    test_domainwatcher.cpp
    test_flowmanager.cpp
    test_flows.cpp
    test_time.cpp
    test_sharedmem.cpp
    Utils.cpp
    ../src/internal/DomainWatcher.cpp
    ../src/internal/Flow.cpp
    ../src/internal/FlowManager.cpp
    ../src/internal/FlowParser.cpp
    ../src/internal/PosixFlowReader.cpp
    ../src/internal/Time.cpp
    ../src/internal/Sync.cpp
)

# Specify the include directories for the test executable to find the library headers
target_include_directories(mxl-tests PRIVATE ${PROJECT_SOURCE_DIR}/lib/include)

if(APPLE)
    target_link_libraries(mxl-tests PRIVATE mxl stduuid Catch2::Catch2WithMain spdlog::spdlog fmt::fmt picojson::picojson)
else()
    target_link_libraries(mxl-tests PRIVATE mxl stduuid Catch2::Catch2WithMain spdlog::spdlog fmt::fmt picojson::picojson PcapPlusPlus::Pcap++)
endif()

# Enable testing using CTest
include(CTest)
include(Catch)
catch_discover_tests(mxl-tests)

# Copy test files to the build directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
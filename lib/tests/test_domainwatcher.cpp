#include "../src/internal/DomainWatcher.hpp"

#include <catch2/catch_test_macros.hpp>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>
#include <uuid.h>

using namespace mxl::lib;
namespace fs = std::filesystem;

TEST_CASE( "Directory Watcher", "[directory watcher]" )
{
    const char *homeDir = getenv( "HOME" );
    REQUIRE( homeDir != nullptr );
    fs::path domain{ std::string{ homeDir } + "/mxl_domain" }; // Remove that path if it exists.
    fs::remove_all( domain );

    DomainWatcher watcher{ domain.string(), nullptr };

    fs::create_directories( domain );

    const auto *id = "5fbec3b1-1b0f-417d-9059-8b94a47197ed";
    auto flowId = *uuids::uuid::from_string( id );
    auto f1Path = domain / id;
    std::ofstream f1{ f1Path };
    auto f2Path = domain / ( id + std::string( ".access" ) );
    std::ofstream f2( f2Path );

    REQUIRE( 1 == watcher.addFlow( flowId, WatcherType::READER ) );
    REQUIRE( 2 == watcher.addFlow( flowId, WatcherType::READER ) );
    REQUIRE( 1 == watcher.addFlow( flowId, WatcherType::WRITER ) );
    REQUIRE( 2 == watcher.addFlow( flowId, WatcherType::WRITER ) );
    REQUIRE( 1 == watcher.removeFlow( flowId, WatcherType::READER ) );
    REQUIRE( 0 == watcher.removeFlow( flowId, WatcherType::READER ) );
    REQUIRE( -1 == watcher.removeFlow( flowId, WatcherType::READER ) );
    REQUIRE( 1 == watcher.removeFlow( flowId, WatcherType::WRITER ) );
    REQUIRE( 0 == watcher.removeFlow( flowId, WatcherType::WRITER ) );
    REQUIRE( -1 == watcher.removeFlow( flowId, WatcherType::WRITER ) );
}

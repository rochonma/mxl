#include "../src/internal/FlowManager.hpp"
#include "Utils.hpp"

#include <catch2/catch_test_macros.hpp>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <memory>
#include <string>
#include <uuid.h>

using namespace mxl::lib;
namespace fs = std::filesystem;

TEST_CASE( "Flow Manager", "[flow manager]" )
{
    const char *homeDir = getenv( "HOME" );
    REQUIRE( homeDir != nullptr );
    fs::path domain{ std::string{ homeDir } + "/mxl_domain" }; // Remove that path if it exists.
    fs::remove_all( domain );

    auto flowDef = mxl::tests::readFile( "data/v210_flow.json" );
    auto flowId = *uuids::uuid::from_string( "5fbec3b1-1b0f-417d-9059-8b94a47197ed" );

    // This should throw since the folder should not exist.
    REQUIRE_THROWS( [&]() { auto manager = std::make_shared<FlowManager>( domain ); }() );

    // Create the mxl domain path.
    REQUIRE( fs::create_directory( domain ) );

    auto manager = std::make_shared<FlowManager>( domain );

    FlowData::ptr flowData;
    Rational grainRate{ 60000, 1001 };

    flowData = manager->createFlow( flowId, flowDef, 5, grainRate, 1024 );

    REQUIRE( flowData != nullptr );
    REQUIRE( flowData->flow != nullptr );
    REQUIRE( flowData->grains.size() == 5 );

    REQUIRE( fs::exists( domain / uuids::to_string( flowId ) ) );

    // Count the grains.
    auto grainDir = domain / ( uuids::to_string( flowId ) + ".grains" );
    size_t grainCount = 0;
    if ( fs::exists( grainDir ) && fs::is_directory( grainDir ) )
    {
        for ( const auto &entry : fs::directory_iterator( grainDir ) )
        {
            if ( fs::is_regular_file( entry ) )
            {
                ++grainCount;
            }
        }
    }
    REQUIRE( grainCount == 5 );

    // This should throw since the flow metadata will already exist.
    REQUIRE_THROWS( [&]() { manager->createFlow( flowId, flowDef, 5, grainRate, 1024 ); }() );

    REQUIRE( manager->listFlows().size() == 1 );

    // Close the flow.  it should not be available for a get operation after being closed.
    manager->closeFlow( flowData );

    REQUIRE( manager->listFlows().size() == 1 );

    // Delete the flow.
    manager->deleteFlow( flowId, flowData );

    REQUIRE( manager->listFlows().size() == 0 );

    // Confirm that files on disk do not exist anymore
    REQUIRE( !fs::exists( domain / uuids::to_string( flowId ) ) );
    REQUIRE( !fs::exists( domain / ( uuids::to_string( flowId ) + ".grains" ) ) );
    REQUIRE( !fs::exists( domain / ( uuids::to_string( flowId ) + ".json" ) ) );
}

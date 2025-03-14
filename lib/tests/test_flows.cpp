#include "../src/internal/Time.hpp"
#include "Utils.hpp"

#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <mxl/flow.h>
#include <mxl/mxl.h>
#include <mxl/time.h>
#include <thread>

namespace fs = std::filesystem;

TEST_CASE( "Flow : Create/Destroy", "[mxl flows]" )
{
    const char *opts = "{}";
    auto flowDef = mxl::tests::readFile( "data/v210_flow.json" );
    const char *flowId = "5fbec3b1-1b0f-417d-9059-8b94a47197ed";

    const char *homeDir = getenv( "HOME" );
    REQUIRE( homeDir != nullptr );
    fs::path domain{ homeDir }; // Remove that path if it exists.
    domain /= "mxl_domain";
    fs::remove_all( domain );

    std::filesystem::create_directories( domain );
    auto instanceReader = mxlCreateInstance( domain.string().c_str(), opts );
    REQUIRE( instanceReader != nullptr );

    auto instanceWriter = mxlCreateInstance( domain.string().c_str(), opts );
    REQUIRE( instanceWriter != nullptr );

    REQUIRE( mxlCreateFlow( instanceWriter, flowDef.c_str(), opts ) == MXL_STATUS_OK );

    mxlFlowReader reader;
    REQUIRE( mxlCreateFlowReader( instanceReader, flowId, "", &reader ) == MXL_STATUS_OK );

    mxlFlowWriter writer;
    REQUIRE( mxlCreateFlowWriter( instanceWriter, flowId, "", &writer ) == MXL_STATUS_OK );

    /// Compute the grain index for the flow rate and current TAI time.
    Rational rate{ 30000, 1001 };
    timespec ts;
    mxlGetTime( &ts );
    uint64_t index = mxlTimeSpecToGrainIndex( &rate, &ts );
    REQUIRE( index != MXL_UNDEFINED_OFFSET );

    /// Open the grain.
    GrainInfo gInfo;
    uint8_t *buffer = nullptr;
    /// Open the grain for writing.
    REQUIRE( mxlFlowWriterOpenGrain( instanceWriter, writer, index, &gInfo, &buffer ) == MXL_STATUS_OK );

    /// Set a mark at the beginning and the end of the grain payload.
    buffer[0] = 0xCA;
    buffer[gInfo.grainSize - 1] = 0xFE;

    /// Get some info about the freshly created flow.  Since no grains have been commited, the head should still be at 0.
    FlowInfo fInfo1;
    REQUIRE( mxlFlowReaderGetInfo( instanceReader, reader, &fInfo1 ) == MXL_STATUS_OK );
    REQUIRE( fInfo1.headIndex == 0 );

    /// Mark the grain as invalid
    gInfo.flags |= GRAIN_FLAG_INVALID;
    REQUIRE( mxlFlowWriterCommit( instanceWriter, writer, &gInfo ) == MXL_STATUS_OK );

    /// Create back the grain using a flow reader.
    REQUIRE( mxlFlowReaderGetGrain( instanceReader, reader, index, 0, &gInfo, &buffer ) == MXL_STATUS_OK );

    std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
    /// Confirm that the flags are preserved.
    REQUIRE( gInfo.flags == GRAIN_FLAG_INVALID );

    /// Confirm that the marks are still present.
    REQUIRE( buffer[0] == 0xCA );
    REQUIRE( buffer[gInfo.grainSize - 1] == 0xFE );

    /// Get the updated flow info
    FlowInfo fInfo2;
    REQUIRE( mxlFlowReaderGetInfo( instanceReader, reader, &fInfo2 ) == MXL_STATUS_OK );

    /// Confirm that that head has moved.
    REQUIRE( fInfo2.headIndex == index );

    // We accessed the grain using mxlFlowReaderGetGrain. This should have increased the lastReadTime field.
    REQUIRE( fInfo2.lastReadTime > fInfo1.lastReadTime );

    // We commited a new grain. This should have increased the lastWriteTime field.
    REQUIRE( fInfo2.lastWriteTime > fInfo1.lastWriteTime );

    /// Delete the reader
    REQUIRE( mxlDestroyFlowReader( instanceReader, reader ) == MXL_STATUS_OK );

    // Use the writer after closing the reader.
    buffer = nullptr;
    REQUIRE( mxlFlowWriterOpenGrain( instanceWriter, writer, index++, &gInfo, &buffer ) == MXL_STATUS_OK );
    /// Set a mark at the beginning and the end of the grain payload.
    buffer[0] = 0xCA;
    buffer[gInfo.grainSize - 1] = 0xFE;

    REQUIRE( mxlDestroyFlowWriter( instanceWriter, writer ) == MXL_STATUS_OK );
    REQUIRE( mxlDestroyFlow( instanceWriter, flowId ) == MXL_STATUS_OK );
    // This should be gone from the filesystem.
    REQUIRE( mxlDestroyFlow( instanceWriter, flowId ) == MXL_ERR_FLOW_NOT_FOUND );

    mxlDestroyInstance( instanceReader );
    mxlDestroyInstance( instanceWriter );
}
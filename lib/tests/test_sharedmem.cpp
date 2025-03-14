#include "../src/internal/Flow.hpp"
#include "../src/internal/SharedMem.hpp"

#include <catch2/catch_test_macros.hpp>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <string>

using namespace mxl::lib;
namespace fs = std::filesystem;

TEST_CASE( "Create", "[shared mem]" )
{
    const char *homeDir = getenv( "HOME" );
    REQUIRE( homeDir != nullptr );
    fs::path p{ std::string{ homeDir } + "/test_shmem" }; // Remove that path if it exists.
    fs::remove( p );

    SharedMem<Flow> reader, writer;

    // Try to open a non-existing file without automatically creating it.
    REQUIRE( false == reader.open( p, false, AccessMode::READ_ONLY ) );

    // Now try to open it with the create mode set to true
    REQUIRE( true == writer.open( p, true, AccessMode::READ_WRITE ) );

    // Confirm that it exists.
    REQUIRE( std::filesystem::exists( p ) );

    // Try to open again the existing file.
    REQUIRE( true == reader.open( p, false, AccessMode::READ_ONLY ) );

    // Confirm that the file is the right size.
    REQUIRE( std::filesystem::file_size( p ) == sizeof( Flow ) );

    auto writer_data = writer.get();
    auto reader_data = reader.get();

    writer_data->info.headIndex = 99;

    // Confirm that the data is shared between the two objects.
    REQUIRE( reader_data->info.headIndex == 99 );

    REQUIRE_NOTHROW( reader.close() );
    REQUIRE_NOTHROW( writer.close() );

    // Final cleanup
    if ( std::filesystem::exists( p ) )
    {
        fs::remove( p );
    }
}

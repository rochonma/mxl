#include "../../lib/src/internal/FlowParser.hpp"

#include <CLI/CLI.hpp>
#include <csignal>
#include <filesystem>
#include <iostream>
#include <minizip-ng/unzip.h>
#include <mxl/flow.h>
#include <mxl/mxl.h>
#include <mxl/time.h>
#include <sstream>
#include <string>
#include <time.h>
#include <uuid.h>
#include <vector>

std::sig_atomic_t volatile g_exit_requested = 0;

void
signal_handler( int )
{
    g_exit_requested = 1;
}

bool
extractFileFromZip( unzFile zipFile, const std::string &targetFile, std::vector<uint8_t> &outData )
{
    if ( unzLocateFile( zipFile, targetFile.c_str(), 0 ) != UNZ_OK )
    {
        std::cerr << "File not found in ZIP: " << targetFile << std::endl;
        return false;
    }

    unz_file_info fileInfo;
    char filename[256];

    if ( unzGetCurrentFileInfo( zipFile, &fileInfo, filename, sizeof( filename ), nullptr, 0, nullptr, 0 ) != UNZ_OK )
    {
        std::cerr << "Failed to get file info for: " << targetFile << std::endl;
        return false;
    }

    if ( unzOpenCurrentFile( zipFile ) != UNZ_OK )
    {
        std::cerr << "Failed to open file: " << targetFile << std::endl;
        return false;
    }

    outData.resize( fileInfo.uncompressed_size );
    if ( unzReadCurrentFile( zipFile, outData.data(), outData.size() ) < 0 )
    {
        std::cerr << "Failed to read file data: " << targetFile << std::endl;
        unzCloseCurrentFile( zipFile );
        return false;
    }

    unzCloseCurrentFile( zipFile );
    return true;
}

bool
extractZipToMemory( const std::string &zipFilePath, std::vector<uint8_t> &jsonData, std::vector<uint8_t> &grainData )
{
    unzFile zipFile = unzOpen( zipFilePath.c_str() );
    if ( !zipFile )
    {
        std::cerr << "Failed to open archive: " << zipFilePath << std::endl;
        return false;
    }

    bool foundJson = extractFileFromZip( zipFile, "flow.json", jsonData );
    if ( !foundJson )
    {
        std::cerr << "flow.json missing from archive" << std::endl;
    }
    bool foundV210 = extractFileFromZip( zipFile, "grain.v210", grainData );
    if ( !foundV210 )
    {
        std::cerr << "grain.v210 missing from archive" << std::endl;
    }

    unzClose( zipFile );
    return foundJson && foundV210;
}

int
play( const std::string &in_domain, const std::string &in_flow, Rational in_rate, const uuids::uuid &in_id, const std::vector<uint8_t> &in_grain )
{
    int status = EXIT_SUCCESS;
    auto flowId = uuids::to_string( in_id );
    uint64_t index = 0;

    // Create the sdk instance
    auto instance = mxlCreateInstance( in_domain.c_str(), "" );
    if ( instance == nullptr )
    {
        std::cerr << "Failed to create MXL instance" << std::endl;
        status = EXIT_FAILURE;
        goto cleanup;
    }

    // Create the flow
    if ( mxlCreateFlow( instance, in_flow.c_str(), "" ) != MXL_STATUS_OK )
    {
        std::cerr << "Failed to create flow" << std::endl;
        status = EXIT_FAILURE;
        goto cleanup;
    }

    // Create the flow writer
    mxlFlowWriter writer;
    if ( mxlCreateFlowWriter( instance, flowId.c_str(), "", &writer ) != MXL_STATUS_OK )
    {
        std::cerr << "Failed to create flow writer" << std::endl;
        status = EXIT_FAILURE;
        goto cleanup;
    }

    index = mxlGetCurrentGrainIndex( &in_rate );
    while ( !g_exit_requested )
    {
        /// Open the grain.
        GrainInfo gInfo;
        uint8_t *buffer = nullptr;
        /// Open the grain for writing.
        if ( mxlFlowWriterOpenGrain( instance, writer, index, &gInfo, &buffer ) != MXL_STATUS_OK )
        {
            std::cerr << "Failed to open grain at index " << index << std::endl;
            break;
        }

        ::memcpy( buffer, in_grain.data(), std::min<uint64_t>( in_grain.size(), gInfo.grainSize ) );

        if ( mxlFlowWriterCommit( instance, writer, &gInfo ) != MXL_STATUS_OK )
        {
            std::cerr << "Failed to open grain at index " << index << std::endl;
            break;
        }
        else
        {
            std::cout << "Commited : " << index << std::endl;
        }

        auto ns = mxlGetNsUntilGrainIndex( index++, &in_rate );
        mxlSleepForNs( ns );
    }

cleanup:
    if ( instance != nullptr )
    {
        mxlDestroyFlowWriter( instance, writer );
        mxlDestroyFlow( instance, flowId.c_str() );
        mxlDestroyInstance( instance );
    }

    return status;
}

int
main( int argc, char **argv )
{
    std::signal( SIGINT, &signal_handler );
    std::signal( SIGTERM, &signal_handler );

    CLI::App app( "mxl-clip-player" );

    mxlVersionType version;
    mxlGetVersion( &version );

    std::stringstream ss;
    ss << version.major << "." << version.minor << "." << version.bugfix << "." << version.build;
    app.set_version_flag( "--version", ss.str() );

    std::string domain;
    auto domainOpt = app.add_option( "-d,--domain", domain, "The MXL domain directory" );
    domainOpt->required( true );
    domainOpt->check( CLI::ExistingDirectory );

    std::string input;
    auto inputOpt = app.add_option( "-i,--input", input, "The input file" );
    inputOpt->check( CLI::ExistingFile );
    inputOpt->required( true );

    CLI11_PARSE( app, argc, argv );

    int status = EXIT_SUCCESS;

    std::vector<uint8_t> json, grain;
    bool extracted = extractZipToMemory( input, json, grain );
    if ( !extracted )
    {
        return EXIT_FAILURE;
    }

    try
    {
        std::cout << "Grain size : " << grain.size() << std::endl;
        std::string flow{ std::begin( json ), std::end( json ) };
        mxl::lib::FlowParser parser{ flow };
        std::cout << "Flow Id    : " << uuids::to_string( parser.getId() ) << std::endl;
        std::cout << "Resolution : " << (uint64_t)parser.get<double>( "frame_width" ) << "x" << (uint64_t)parser.get<double>( "frame_height" )
                  << std::endl;

        Rational r = parser.getGrainRate();
        std::cout << "Grain Rate :" << r.numerator << "/" << r.denominator << std::endl;
        status = play( domain, flow, r, parser.getId(), grain );
    }
    catch ( std::exception &e )
    {
        std::cerr << e.what() << std::endl;
    }

    return status;
}
#include "FlowManager.hpp"

#include "Flow.hpp"
#include "FlowParser.hpp"
#include "Logging.hpp"
#include "SharedMem.hpp"

#include <cstddef>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <ios>
#include <memory>
#include <mxl/flow.h>
#include <stdexcept>
#include <string>
#include <system_error>
#include <uuid.h>
#include <vector>

namespace mxl::lib {

namespace fs = std::filesystem;

FlowManager::FlowManager( const std::filesystem::path &in_mxlDomain ) : _mxlDomain{ in_mxlDomain }
{
    if ( !fs::exists( in_mxlDomain ) || !fs::is_directory( in_mxlDomain ) )
    {
        throw fs::filesystem_error(
            "Path does not exist or is not a directory", in_mxlDomain, std::make_error_code( std::errc::no_such_file_or_directory ) );
    }
}

FlowData::ptr
FlowManager::createFlow( const uuids::uuid &in_flowId, const std::string &in_flowDef, size_t in_grainCount, size_t in_grainPayloadSize )
{
    auto uuid = uuids::to_string( in_flowId );
    MXL_DEBUG( "Create flow. id: {}, grainCount: {}, grain payload size: {}", uuid, in_grainCount, in_grainPayloadSize );

    // Parse the flow early.  if the flow is invalid it will throw.
    FlowParser parser{ in_flowDef };

    auto flowFile = _mxlDomain / uuid;
    if ( fs::exists( flowFile ) )
    {
        throw fs::filesystem_error( "Flow file conflict. File already exists", flowFile, std::make_error_code( std::errc::file_exists ) );
    }

    auto flowJsonFile = _mxlDomain / ( uuid + ".json" );
    if ( fs::exists( flowJsonFile ) )
    {
        throw fs::filesystem_error(
            "Flow description file conflict. File already exists", flowJsonFile, std::make_error_code( std::errc::file_exists ) );
    }

    auto grainDir = _mxlDomain / ( uuid + ".grains" );
    if ( fs::exists( grainDir ) )
    {
        throw fs::filesystem_error( "Grain directory conflict. Directory already exists", grainDir, std::make_error_code( std::errc::file_exists ) );
    }

    // Write the json file to disk.
    std::ofstream out{ flowJsonFile, std::ios::out | std::ios::trunc };
    if ( !out )
    {
        throw fs::filesystem_error( "Failed to open file", flowJsonFile, std::make_error_code( std::errc::io_error ) );
    }
    else
    {
        out << in_flowDef;
        out.close();
    }

    auto readAccessFile = _mxlDomain / ( uuid + ".access" );
    if ( fs::exists( readAccessFile ) )
    {
        throw fs::filesystem_error( "Read access file already exists", readAccessFile, std::make_error_code( std::errc::file_exists ) );
    }
    else
    {
        // create the dummy file.
        std::ofstream out{ readAccessFile, std::ios::out | std::ios::trunc };
    }

    auto flowData = std::make_shared<FlowData>();
    flowData->flow = std::make_shared<SharedMem<Flow>>();

    if ( !flowData->flow->open( flowFile.string(), true, AccessMode::READ_WRITE ) )
    {
        throw std::runtime_error( "Failed to create flow shared memory." );
    }

    auto &info = flowData->flow->get()->info;
    info.version = 1;
    info.size = sizeof( FlowInfo );
    info.grainRate = parser.getGrainRate();
    info.grainCount = in_grainCount;

    auto idSpan = in_flowId.as_bytes();
    std::memcpy( info.id, idSpan.data(), idSpan.size() );

    fs::create_directory( grainDir );
    for ( size_t i = 0; i < in_grainCount; i++ )
    {
        auto grain = std::make_shared<SharedMem<Grain>>();
        auto grainPath = grainDir / std::to_string( i );

        MXL_TRACE( "Creating grain: {}", grainPath.string() );

        // \todo Handle payload stored device memory
        size_t totalSize = MXL_GRAIN_PAYLOAD_OFFSET + in_grainPayloadSize;
        if ( !grain->open( grainPath.string(), true, AccessMode::READ_WRITE, totalSize ) )
        {
            throw std::runtime_error( "Failed to create grain shared memory." );
        }

        auto &gInfo = grain->get()->info;
        gInfo.grainSize = in_grainPayloadSize;
        gInfo.version = 1;
        gInfo.size = sizeof( GrainInfo );
        gInfo.deviceIndex = -1;
        flowData->grains.push_back( grain );
    }

    return flowData;
}

FlowData::ptr
FlowManager::openFlow( const uuids::uuid &in_flowId, AccessMode in_mode )
{
    auto uuid = uuids::to_string( in_flowId );

    fs::path base{ _mxlDomain };

    // Verify that the flow file exists.
    auto flowFile = base / uuid;
    if ( !fs::exists( flowFile ) )
    {
        throw fs::filesystem_error( "Flow file not found", flowFile, std::make_error_code( std::errc::no_such_file_or_directory ) );
    }

    // Open the shared memory
    auto flowData = std::make_shared<FlowData>();
    flowData->flow = std::make_shared<SharedMem<Flow>>();
    if ( !flowData->flow->open( flowFile.string(), false, in_mode ) )
    {
        throw std::runtime_error( "Failed to open flow shared memory." );
    }

    size_t grainCount = flowData->flow->get()->info.grainCount;
    auto grainDir = base / ( uuid + ".grains" );
    if ( fs::exists( grainDir ) && fs::is_directory( grainDir ) )
    {
        // Open each grains
        for ( size_t i = 0; i < grainCount; i++ )
        {
            auto grain = std::make_shared<SharedMem<Grain>>();
            auto grainPath = grainDir / std::to_string( i );
            MXL_TRACE( "Opening grain: {}", grainPath.string() );
            if ( !grain->open( grainPath.string(), false, in_mode ) )
            {
                throw std::runtime_error( "Failed to open grain shared memory." );
            }
            flowData->grains.push_back( grain );
        }
    }
    else
    {
        throw fs::filesystem_error( "Grain directory not found.", grainDir, std::make_error_code( std::errc::no_such_file_or_directory ) );
    }

    return flowData;
}

void
FlowManager::closeFlow( FlowData::ptr in_flowData )
{
    // Explicitly close all resources.
    in_flowData->flow->close();
    for ( auto grain : in_flowData->grains )
    {
        grain->close();
    }

    in_flowData->flow.reset();
    in_flowData->grains.clear();
}

bool
FlowManager::deleteFlow( const uuids::uuid &in_flowId, FlowData::ptr in_flowData )
{
    auto uuid = uuids::to_string( in_flowId );
    MXL_TRACE( "Delete flow: {}", uuid );

    // First. close the flow if opened.
    if ( in_flowData && in_flowData->flow )
    {
        closeFlow( in_flowData );
    }

    fs::path base{ _mxlDomain };
    bool found = std::filesystem::remove( base / uuid );
    std::filesystem::remove( base / ( uuid + ".json" ) );
    std::filesystem::remove( base / ( uuid + ".access" ) );
    std::filesystem::remove_all( base / ( uuid + ".grains" ) );

    return found;
}

void
FlowManager::garbageCollect()
{
    MXL_WARN( "Garbage collection of flows not implemented yet." );
    /// \todo Implement a garbage collection low priority thread.
}

std::vector<uuids::uuid>
FlowManager::listFlows() const
{
    fs::path base{ _mxlDomain };

    std::vector<uuids::uuid> flowIds;

    if ( fs::exists( base ) && fs::is_directory( base ) )
    {
        for ( const auto &entry : fs::directory_iterator( _mxlDomain ) )
        {
            if ( fs::is_regular_file( entry ) && entry.path().extension().empty() )
            {
                // this looks like a uuid. try to parse it an confirm it is valid.
                auto id = uuids::uuid::from_string( entry.path().filename().string() );
                if ( id.has_value() )
                {
                    flowIds.push_back( *id );
                }
            }
        }
    }
    else
    {
        throw fs::filesystem_error( "Base directory not found.", base, std::make_error_code( std::errc::no_such_file_or_directory ) );
    }

    return flowIds;
}

FlowManager::~FlowManager()
{
    MXL_TRACE( "~FlowManager" );
}

}
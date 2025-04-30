#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <iomanip>
#include <ios>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <uuid.h>
#include <CLI/CLI.hpp>
#include <gsl/span>
#include <mxl/flow.h>
#include <mxl/mxl.h>
#include <mxl/time.h>

namespace fs = std::filesystem;

namespace
{

    double timespec_diff_ms(const struct timespec& start, const struct timespec& end)
    {
        int64_t diff_ns = (end.tv_sec - start.tv_sec) * 1000000000LL + (end.tv_nsec - start.tv_nsec);
        return diff_ns / 1e6;
    }

} // namespace

std::ostream& operator<<(std::ostream& os, timespec const& ts)
{
    std::time_t sec = ts.tv_sec;
    struct tm tm;
    gmtime_r(&sec, &tm);
    os << std::put_time(&tm, "%Y-%m-%d %H:%M:%S UTC") << " +" << std::setw(9) << std::setfill('0') << ts.tv_nsec << "ns";
    return os;
}

std::ostream& operator<<(std::ostream& os, FlowInfo const& info)
{
    auto span = gsl::span<uint8_t, 16>(const_cast<uint8_t*>(info.id), sizeof(info.id));
    auto id = uuids::uuid(span);
    os << "- Flow [" << uuids::to_string(id) << "]" << std::endl;
    os << "\tversion           : " << info.version << std::endl;
    os << "\tstruct size       : " << info.size << std::endl;
    os << "\tflags             : 0x" << std::setw(32) << std::setfill('0') << std::hex << info.flags << std::dec << std::endl;
    os << "\thead index        : " << info.headIndex << std::endl;
    os << "\tgrain rate        : " << info.grainRate.numerator << "/" << info.grainRate.denominator << std::endl;
    os << "\tgrain count       : " << info.grainCount << std::endl;
    os << "\tlast write time   : " << info.lastWriteTime << std::endl;
    os << "\tlast read time    : " << info.lastReadTime << std::endl;

    timespec ts;
    mxlGetTime(&ts);
    auto currentIndex = mxlTimeSpecToGrainIndex(&info.grainRate, &ts);
    os << "\tLatency (grains)  : " << (currentIndex - info.headIndex) << std::endl;
    os << "\tLatency (ms)      : " << std::fixed << std::setprecision(3) << timespec_diff_ms(info.lastWriteTime, ts) << std::endl;

    return os;
}

int listAllFlows(std::string const& in_domain)
{
    std::cout << "--- MXL Flows ---" << std::endl;
    fs::path base{in_domain};

    if (fs::exists(base) && fs::is_directory(base))
    {
        for (auto const& entry : fs::directory_iterator(base))
        {
            if (fs::is_regular_file(entry) && entry.path().extension().empty())
            {
                // this looks like a uuid. try to parse it an confirm it is valid.
                auto id = uuids::uuid::from_string(entry.path().filename().string());
                if (id.has_value())
                {
                    std::cout << "\t" << *id << std::endl;
                }
            }
        }
    }

    std::cout << std::endl;
    return EXIT_SUCCESS;
}

int printFlow(std::string const& in_domain, std::string const& in_id)
{
    int ret = EXIT_SUCCESS;

    // Create the SDK instance with a specific domain.
    auto instance = mxlCreateInstance(in_domain.c_str(), "");
    if (instance == nullptr)
    {
        std::cerr << "Failed to create MXL instance" << std::endl;
        return EXIT_FAILURE;
    }

    // Create a flow reader for the given flow id.
    mxlFlowReader reader;
    mxlStatus status = mxlCreateFlowReader(instance, in_id.c_str(), "", &reader);
    if (status != MXL_STATUS_OK)
    {
        std::cerr << "Failed to create flow reader" << std::endl;
        mxlDestroyInstance(instance);
        return EXIT_FAILURE;
    }

    // Extract the FlowInfo structure.
    FlowInfo info;
    status = mxlFlowReaderGetInfo(instance, reader, &info);
    if (status != MXL_STATUS_OK)
    {
        std::cerr << "Failed to get flow info" << std::endl;
        ret = EXIT_FAILURE;
    }
    else
    {
        std::cout << info << std::endl;
        ret = EXIT_SUCCESS;
    }

    // Cleanup
    mxlDestroyFlowReader(instance, reader);
    mxlDestroyInstance(instance);

    return ret;
}

int main(int argc, char** argv)
{
    CLI::App app("mxl-info");

    mxlVersionType version;
    mxlGetVersion(&version);

    std::stringstream ss;
    ss << version.major << "." << version.minor << "." << version.bugfix << "." << version.build;

    // add version output
    app.set_version_flag("--version", ss.str());

    std::string domain = "";
    auto domainOpt = app.add_option("-d,--domain", domain, "The MXL domain directory");
    domainOpt->required(true);
    domainOpt->check(CLI::ExistingDirectory);

    std::string flowId = "";
    auto flowIdOpt = app.add_option("-f,--flow", flowId, "The flow id to analyse");

    auto allOpt = app.add_flag("-l,--list", "List all flows in the MXL domain");

    CLI11_PARSE(app, argc, argv);

    int status = EXIT_SUCCESS;

    if (allOpt->count() > 0)
    {
        status = listAllFlows(domain);
    }
    else if (flowIdOpt->count() > 0)
    {
        status = printFlow(domain, flowId);
    }

    return status;
}
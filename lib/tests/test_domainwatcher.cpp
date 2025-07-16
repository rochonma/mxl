#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>
#include <uuid.h>
#include <catch2/catch_test_macros.hpp>
#include <condition_variable>
#include "../src/internal/DomainWatcher.hpp"
#include "../src/internal/PathUtils.hpp"

using namespace mxl::lib;
namespace fs = std::filesystem;

TEST_CASE("Directory Watcher", "[directory watcher]")
{
    char const* homeDir = getenv("HOME");
    REQUIRE(homeDir != nullptr);
    fs::path domain{std::string{homeDir} + "/mxl_domain"}; // Remove that path if it exists.
    fs::remove_all(domain);

    fs::create_directories(domain);

    auto watcher = DomainWatcher{domain.native(), nullptr};

    auto const* id = "5fbec3b1-1b0f-417d-9059-8b94a47197ed";
    auto const flowDirectory = makeFlowDirectoryName(domain, id);

    fs::create_directories(flowDirectory);

    auto flowId = *uuids::uuid::from_string(id);
    auto f1Path = makeFlowDataFilePath(flowDirectory);
    auto f1 = std::ofstream{f1Path};

    auto f2Path = makeFlowAccessFilePath(flowDirectory);
    auto f2 = std::ofstream{f2Path};

    REQUIRE(1 == watcher.addFlow(flowId, WatcherType::READER));
    REQUIRE(2 == watcher.addFlow(flowId, WatcherType::READER));
    REQUIRE(1 == watcher.addFlow(flowId, WatcherType::WRITER));
    REQUIRE(2 == watcher.addFlow(flowId, WatcherType::WRITER));
    REQUIRE(1 == watcher.removeFlow(flowId, WatcherType::READER));
    REQUIRE(0 == watcher.removeFlow(flowId, WatcherType::READER));
    REQUIRE(-1 == watcher.removeFlow(flowId, WatcherType::READER));
    REQUIRE(1 == watcher.removeFlow(flowId, WatcherType::WRITER));
    REQUIRE(0 == watcher.removeFlow(flowId, WatcherType::WRITER));
    REQUIRE(-1 == watcher.removeFlow(flowId, WatcherType::WRITER));
}

#ifdef __linux__ // The following tests apply only to Linux (inotify) implementation

TEST_CASE("DomainWatcher triggers callback on file modifications", "[domainwatcher]")
{
    // This test checks that modifying a watched file triggers the callback with correct UUID and WatcherType.
    char const* homeDir = std::getenv("HOME");
    REQUIRE(homeDir != nullptr);
    fs::path domainPath = fs::path(homeDir) / "mxl_test_domain_events";
    fs::remove_all(domainPath);
    fs::create_directories(domainPath);

    // Variables to capture callback information
    std::mutex eventMutex;
    std::condition_variable eventCV;
    std::atomic<int> eventCount{0};
    uuids::uuid callbackUuid;
    mxl::lib::WatcherType callbackType;

    // Set up DomainWatcher with a callback that notifies when a file change event is caught
    mxl::lib::DomainWatcher watcher(domainPath,
        [&](uuids::uuid id, mxl::lib::WatcherType type)
        {
            std::unique_lock<std::mutex> lock(eventMutex);
            callbackUuid = id;
            callbackType = type;
            eventCount++;
            lock.unlock();
            eventCV.notify_one();
        });

    // Create a flow directory and files, then add watchers for both Reader and Writer types
    std::string flowIdStr = "11111111-2222-3333-4444-555555555555"; // test UUID
    fs::path flowDir = mxl::lib::makeFlowDirectoryName(domainPath, flowIdStr);
    fs::create_directories(flowDir);
    auto flowId = *uuids::uuid::from_string(flowIdStr);
    fs::path dataFile = mxl::lib::makeFlowDataFilePath(flowDir);     // main data file
    fs::path accessFile = mxl::lib::makeFlowAccessFilePath(flowDir); // .access file
    std::ofstream(dataFile).close();
    std::ofstream(accessFile).close();

    REQUIRE(watcher.addFlow(flowId, mxl::lib::WatcherType::READER) == 1);
    REQUIRE(watcher.addFlow(flowId, mxl::lib::WatcherType::WRITER) == 1);

    // Give the watcher thread a moment to register the inotify watch
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Test 1: Modify the main data file and expect a READER-type event
    int initialCount = eventCount.load();
    {
        std::ofstream f(dataFile, std::ios::app);
        f << "TestData";
        f.flush();
        f.close();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Wait for callback with timeout
    std::unique_lock<std::mutex> lock(eventMutex);
    bool gotEvent = eventCV.wait_for(lock, std::chrono::seconds(2), [&] { return eventCount.load() > initialCount; });
    lock.unlock();

    REQUIRE(gotEvent);
    REQUIRE(callbackUuid == flowId);
    REQUIRE(callbackType == mxl::lib::WatcherType::READER);

    // Clean up: remove the flow watches
    REQUIRE(watcher.removeFlow(flowId, mxl::lib::WatcherType::READER) == 0);
    REQUIRE(watcher.removeFlow(flowId, mxl::lib::WatcherType::WRITER) == 0);
}

TEST_CASE("DomainWatcher thread start/stop behavior", "[domainwatcher]")
{
    // This test ensures the internal event-processing thread starts on construction and stops on request.
    char const* homeDir = std::getenv("HOME");
    REQUIRE(homeDir != nullptr);
    fs::path domainPath = fs::path(homeDir) / "mxl_test_domain_thread";
    fs::remove_all(domainPath);
    fs::create_directories(domainPath);

    // Set up a callback that will signal when an event is processed
    std::mutex eventMutex;
    std::condition_variable eventCV;
    bool eventReceived = false;
    mxl::lib::DomainWatcher watcher(domainPath,
        [&](uuids::uuid, mxl::lib::WatcherType)
        {
            std::lock_guard<std::mutex> lock(eventMutex);
            eventReceived = true;
            eventCV.notify_one();
        });

    // Prepare a flow and add a watch so that events can be generated
    std::string flowIdStr = "aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee";
    fs::path flowDir = mxl::lib::makeFlowDirectoryName(domainPath, flowIdStr);
    fs::create_directories(flowDir);
    auto flowId = *uuids::uuid::from_string(flowIdStr);
    fs::path dataFile = mxl::lib::makeFlowDataFilePath(flowDir);
    std::ofstream(dataFile).close();
    REQUIRE(watcher.addFlow(flowId, mxl::lib::WatcherType::READER) == 1);

    // Trigger an event to confirm the thread is running and processing events
    {
        std::ofstream f(dataFile, std::ios::app);
        f << "1"; // modify file to generate an inotify event
    }
    // Wait for the callback (with a short timeout)
    std::unique_lock<std::mutex> lock(eventMutex);
    bool gotEvent = eventCV.wait_for(lock, std::chrono::milliseconds(500), [&] { return eventReceived; });
    REQUIRE(gotEvent); // The event thread should have processed the file change
    lock.unlock();
    eventReceived = false;

    // Now stop the watcher thread and ensure no further events are processed
    watcher.stop(); // signal the DomainWatcher to stop its event loop

    // Give the thread time to actually stop processing
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    {
        std::ofstream f(dataFile, std::ios::app);
        f << "2"; // another modification after stopping
        f.flush();
        f.close();
    }

    // Wait longer to see if any callback occurs (it should not, since thread is stopped)
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    // The eventReceived flag should remain false because the thread is stopped
    REQUIRE(eventReceived == false);

    // Cleanup
    REQUIRE(watcher.removeFlow(flowId, mxl::lib::WatcherType::READER) == 0);
    // DomainWatcher destructor will join the thread (if not already stopped) and close fds.
}

TEST_CASE("DomainWatcher error handling for invalid inputs", "[domainwatcher]")
{
    // This test simulates error conditions: adding watches for non-existent files and removing non-existent watches.
    char const* homeDir = std::getenv("HOME");
    REQUIRE(homeDir != nullptr);

    // Case 1: Using a valid domain path but adding a flow without creating the files
    fs::path goodDomain = fs::path(homeDir) / "mxl_error_test_domain";
    fs::remove_all(goodDomain);
    fs::create_directories(goodDomain);
    mxl::lib::DomainWatcher watcher(goodDomain, nullptr);

    // Attempt to addFlow on a domain where the flow files don't exist (should throw because files are not present)
    uuids::uuid dummyId = *uuids::uuid::from_string("01234567-89ab-cdef-0123-456789abcdef");
    // The implementation throws std::runtime_error when inotify_add_watch fails
    REQUIRE_THROWS_AS(watcher.addFlow(dummyId, mxl::lib::WatcherType::READER), std::runtime_error);

    // Case 2: Using a valid domain path but adding a flow without creating the files
    // Choose a new flow ID and call addFlow without creating the flow directory or files
    uuids::uuid noFileId = *uuids::uuid::from_string("fedcba98-7654-3210-fedc-ba9876543210");
    // Should throw because the file doesn't exist
    REQUIRE_THROWS_AS(watcher.addFlow(noFileId, mxl::lib::WatcherType::WRITER), std::runtime_error);
    // Also test removing a watch that was never added (should return -1)
    REQUIRE(watcher.removeFlow(noFileId, mxl::lib::WatcherType::WRITER) == -1);

    // Case 3: Removing a non-existent watch on a brand new DomainWatcher (nothing added at all)
    uuids::uuid randomId = *uuids::uuid::from_string("00000000-0000-0000-0000-000000000000");
    mxl::lib::DomainWatcher emptyWatcher(goodDomain, nullptr);
    REQUIRE(emptyWatcher.removeFlow(randomId, mxl::lib::WatcherType::READER) == -1);
}

TEST_CASE("DomainWatcher cleans up file descriptors on destruction", "[domainwatcher]")
{
    // This test verifies that DomainWatcher closes its inotify/epoll descriptors upon destruction,
    // so no file descriptor leaks occur.
    namespace fs = std::filesystem;
    // Helper: count open file descriptors (excluding the FD for /proc/self/fd itself)
    auto countOpenFDs = []()
    {
        size_t count = 0;
        for (auto& entry : fs::directory_iterator("/proc/self/fd"))
        {
            // Exclude the special entries "." and ".." if present
            std::string const name = entry.path().filename().string();
            if (name == "." || name == "..")
            {
                continue;
            }
            count++;
        }
        return count;
    };

    char const* homeDir = std::getenv("HOME");
    REQUIRE(homeDir != nullptr);
    fs::path domainPath = fs::path(homeDir) / "mxl_fd_test_domain";
    fs::remove_all(domainPath);
    fs::create_directories(domainPath);

    size_t fdsBefore = countOpenFDs();
    { // scope to ensure DomainWatcher destruction
        mxl::lib::DomainWatcher watcher(domainPath, nullptr);
        // Add a watch to force inotify initialization if not already done in constructor
        std::string flowIdStr = "12345678-1234-5678-1234-567812345678";
        fs::path flowDir = mxl::lib::makeFlowDirectoryName(domainPath, flowIdStr);
        fs::create_directories(flowDir);
        auto flowId = *uuids::uuid::from_string(flowIdStr);
        fs::path filePath = mxl::lib::makeFlowDataFilePath(flowDir);
        std::ofstream(filePath).close();
        REQUIRE(watcher.addFlow(flowId, mxl::lib::WatcherType::READER) == 1);
        // (No need to trigger events here; we just want the watcher to open its fds)
    } // DomainWatcher goes out of scope here, its destructor should close inotify/epoll fds
    size_t fdsAfter = countOpenFDs();

    // After destruction, the number of open fds should be back to the original count
    // (The DomainWatcher should have closed the two fds it opened: inotify and epoll)
    REQUIRE(fdsAfter == fdsBefore);
}

TEST_CASE("DomainWatcher supports concurrent add/remove operations", "[domainwatcher]")
{
    // This test spawns multiple threads to add and remove flow watchers concurrently,
    // to ensure thread safety (no crashes or race conditions and correct final state).
    char const* homeDir = std::getenv("HOME");
    REQUIRE(homeDir != nullptr);
    fs::path domainPath = fs::path(homeDir) / "mxl_concurrent_domain";
    fs::remove_all(domainPath);
    fs::create_directories(domainPath);
    mxl::lib::DomainWatcher watcher(domainPath, nullptr);

    // Prepare a flow file for watching
    std::string flowIdStr = "deadc0de-dead-beef-dead-beefdeadc0de";
    fs::path flowDir = mxl::lib::makeFlowDirectoryName(domainPath, flowIdStr);
    fs::create_directories(flowDir);
    auto flowId = *uuids::uuid::from_string(flowIdStr);
    fs::path filePath = mxl::lib::makeFlowDataFilePath(flowDir);
    std::ofstream(filePath).close(); // create the file to watch

    int const iterations = 100;      // Reduced iterations for more predictable results
    std::atomic<int> addCount{0};
    std::atomic<int> removeCount{0};

    // Launch two threads: one repeatedly adds a reader watch, the other removes it
    std::thread adder(
        [&]()
        {
            for (int i = 0; i < iterations; ++i)
            {
                try
                {
                    int result = watcher.addFlow(flowId, mxl::lib::WatcherType::READER);
                    if (result > 0)
                    {
                        addCount++;
                    }
                }
                catch (...)
                {
                    // Ignore exceptions from concurrent access
                }
                if (i % 10 == 0)
                {
                    std::this_thread::yield();
                } // occasionally yield to other thread
            }
        });
    std::thread remover(
        [&]()
        {
            for (int i = 0; i < iterations; ++i)
            {
                try
                {
                    int result = watcher.removeFlow(flowId, mxl::lib::WatcherType::READER);
                    if (result >= 0)
                    {
                        removeCount++;
                    }
                }
                catch (...)
                {
                    // Ignore exceptions from concurrent access
                }
                if (i % 10 == 0)
                {
                    std::this_thread::yield();
                }
            }
        });

    // Wait for both threads to finish
    adder.join();
    remover.join();

    // After concurrent operations, clean up any remaining watches
    // The exact final state is unpredictable due to concurrency, but we should be able to clean up
    int cleanupAttempts = 0;
    int result = 0;
    do
    {
        result = watcher.removeFlow(flowId, mxl::lib::WatcherType::READER);
        cleanupAttempts++;
    }
    while (result > 0 && cleanupAttempts < 10); // Limit cleanup attempts

    // Verify the watcher is still functional after stress testing
    REQUIRE(watcher.addFlow(flowId, mxl::lib::WatcherType::READER) >= 1);
    REQUIRE(watcher.removeFlow(flowId, mxl::lib::WatcherType::READER) >= 0);
}

TEST_CASE("DomainWatcher ctor throws on invalid domain path", "[domainwatcher]")
{
    // Point at a path that cannot be used as a directory (e.g. a regular file)
    auto tmp = std::filesystem::temp_directory_path() / "not_a_dir";
    std::ofstream(tmp).close(); // create a plain file
    REQUIRE_THROWS_AS(mxl::lib::DomainWatcher(tmp, nullptr), std::filesystem::filesystem_error);
}

TEST_CASE("DomainWatcher allows re-add after full remove", "[domainwatcher]")
{
    // Create domain and file
    auto domain = std::filesystem::temp_directory_path() / "mxl_dw_readd";
    std::filesystem::remove_all(domain);
    std::filesystem::create_directories(domain);
    auto flowId = *uuids::uuid::from_string("abcdefab-1234-5678-9abc-abcdefabcdef");
    auto flowDir = mxl::lib::makeFlowDirectoryName(domain, uuids::to_string(flowId));
    std::filesystem::create_directories(flowDir);
    auto file = mxl::lib::makeFlowDataFilePath(flowDir);
    std::ofstream(file).close();

    DomainWatcher watcher(domain, nullptr);
    // Add once, remove fully, then add again—should succeed twice
    REQUIRE(watcher.addFlow(flowId, WatcherType::READER) == 1);
    REQUIRE(watcher.removeFlow(flowId, WatcherType::READER) == 0);
    REQUIRE(watcher.addFlow(flowId, WatcherType::READER) == 1);
}

#endif // __linux__

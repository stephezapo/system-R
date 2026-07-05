#include <catch2/catch_test_macros.hpp>

#include "system_r/core/TripleBuffer.h"

#include <atomic>
#include <thread>

using systemr::TripleBuffer;

TEST_CASE ("acquire returns false until something is published", "[triplebuffer]")
{
    TripleBuffer<int> tb;
    REQUIRE_FALSE (tb.acquire());
}

TEST_CASE ("consumer sees the most recently published value", "[triplebuffer]")
{
    TripleBuffer<int> tb;

    tb.writeSlot() = 11;
    tb.publish();
    tb.writeSlot() = 22;
    tb.publish();

    REQUIRE (tb.acquire());
    REQUIRE (tb.readSlot() == 22);     // latest wins; the 11 was superseded
    REQUIRE_FALSE (tb.acquire());      // nothing new since
}

TEST_CASE ("writer and reader never alias under concurrency", "[triplebuffer]")
{
    TripleBuffer<int> tb;
    std::atomic<bool> stop { false };
    std::atomic<int> lastSeen { 0 };
    bool monotonic = true;

    std::thread producer ([&]
    {
        for (int i = 1; i <= 200000; ++i)
        {
            tb.writeSlot() = i;
            tb.publish();
        }
        stop = true;
    });

    while (! stop)
    {
        if (tb.acquire())
        {
            const int v = tb.readSlot();
            if (v < lastSeen.load())    // published values only ever increase
                monotonic = false;
            lastSeen = v;
        }
    }

    producer.join();
    REQUIRE (monotonic);
}

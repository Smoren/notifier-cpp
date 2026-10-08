#include "app.h"

#include <atomic>
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <thread>

#include "notifier.h"

using std::chrono::milliseconds;
using std::chrono::steady_clock;

namespace {

// Producer thread: tries to send notifications as fast as it can. The
// min_wait_micro limit in the config throttles the ones that come too often.
void producer(Notifier& notifier, const int attempts, std::atomic<int>& sent) {
    for (int i = 0; i < attempts; ++i) {
        if (notifier.notify()) {
            ++sent;
            std::cout << "   [producer] notify #" << (i + 1) << " -> sent\n";
        } else {
            std::cout << "   [producer] notify #" << (i + 1) << " -> throttled by min_wait_micro\n";
        }
        std::this_thread::sleep_for(milliseconds(30));
    }
    std::cout << "   [producer] done, sent " << sent.load() << "/" << attempts << "\n";
}

// Consumer thread: blocks in wait(). It returns either on a notification or
// when max_wait_micro expires (watchdog), whichever comes first. Since wait()
// is edge-triggered and returns void, the wake-up cause is inferred from the
// elapsed time: close to max_wait_micro means a timeout, otherwise a
// notification arrived.
void consumer(Notifier& notifier, const int iterations, const int max_wait_micro) {
    for (int i = 0; i < iterations; ++i) {
        const auto start = steady_clock::now();
        notifier.wait();
        const auto elapsed = std::chrono::duration_cast<milliseconds>(steady_clock::now() - start);

        const bool timed_out = elapsed >= milliseconds(max_wait_micro / 1000);
        std::cout << "   [consumer] wait() #" << (i + 1) << " returned after ~" << elapsed.count()
                  << " ms (" << (timed_out ? "max_wait_micro timeout" : "notification") << ")\n";
    }
}

} // namespace

void run_app() {
    std::cout << "=== Notifier two-thread demo ===\n\n";

    Notifier::NotifierConfig config;
    config.min_wait_micro = 100'000; // 100 ms — notifications cannot come faster
    config.max_wait_micro = 250'000; // 250 ms — wait() returns at most after this

    std::cout << "config: min_wait_micro = 100 ms, max_wait_micro = 250 ms\n";
    std::cout << "producer sends every 30 ms, consumer waits in a loop\n\n";

    Notifier notifier(config);
    std::atomic<int> sent{0};

    std::thread producer_thread(producer, std::ref(notifier), 6, std::ref(sent));
    std::thread consumer_thread(consumer, std::ref(notifier), 4, config.max_wait_micro.value());

    producer_thread.join();
    consumer_thread.join();

    std::cout << "\n";

    // Explicit wait(rel_time_micro) overload: an explicit timeout per call,
    // still bounded below by min_wait_micro.
    std::cout << "explicit wait(rel_time_micro) with min_wait_micro = 100 ms:\n";
    const auto start = steady_clock::now();
    notifier.wait(150'000); // 150 ms >= min_wait_micro -> allowed
    const auto elapsed = std::chrono::duration_cast<milliseconds>(steady_clock::now() - start);
    std::cout << "   wait(150 ms) returned after ~" << elapsed.count() << " ms\n";

    try {
        notifier.wait(50'000); // 50 ms < min_wait_micro -> throws
        std::cout << "   wait(50 ms) -> no exception (unexpected)\n";
    } catch (const std::invalid_argument& e) {
        std::cout << "   wait(50 ms) -> std::invalid_argument: " << e.what() << "\n";
    }

    std::cout << "\n=== done ===\n";
}

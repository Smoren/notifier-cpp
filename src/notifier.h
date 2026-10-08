#pragma once

#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <optional>

// Thread-safe notifier built on top of std::condition_variable.
//
// A producer calls notify(); consumers call wait(). One notification wakes
// all threads currently waiting.
//
// The config limits the notification rate:
//   - min_wait_micro: minimum period between two consecutive notifications,
//                     in microseconds. notify() returns false if it is called
//                     earlier than min_wait_micro after the previous
//                     successful notification.
//   - max_wait_micro: maximum time wait() blocks without a notification,
//                     in microseconds. wait() returns after this timeout even
//                     if no notification has arrived (watchdog / periodic
//                     wake-up).
class Notifier {
public:
    struct NotifierConfig {
        std::optional<int> min_wait_micro; // microseconds, lower bound between notifications
        std::optional<int> max_wait_micro; // microseconds, upper bound for a single wait()
    };

    explicit Notifier(const NotifierConfig& config = {});

    Notifier(const Notifier&) = delete;
    Notifier& operator=(const Notifier&) = delete;

    // Sends a notification to all waiting threads if the config allows it.
    // Returns true if the notification was sent, false if it was throttled
    // by min_wait_micro.
    bool notify();

    // Blocks until the next notification (edge-triggered: notifications sent
    // before the call are not queued). If max_wait_micro is set, returns after
    // at most that time even if no notification has arrived.
    void wait();

    // Blocks until the next notification or until rel_time_micro microseconds
    // have elapsed, whichever comes first. The explicit timeout overrides
    // max_wait_micro from the config for this call. Only min_wait_micro is
    // taken into account: if it is set and rel_time_micro is smaller than it,
    // std::invalid_argument is thrown.
    void wait(int rel_time_micro);

private:
    NotifierConfig _config;

    std::mutex _mutex;
    std::condition_variable _cv;

    // Monotonic counter incremented on every successful notification. Waiters
    // capture it before blocking and wake up once it changes.
    std::uint64_t _generation = 0;

    // Time of the last successful notification (used by min_wait_micro throttling).
    std::chrono::steady_clock::time_point _last_notify{};
    bool _has_notified = false;
};

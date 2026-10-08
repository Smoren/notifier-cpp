#include "tools.h"

#include <chrono>
#include <future>

double get_current_timestamp() {
    // Get the current time as the number of nanoseconds since the epoch
    const auto now = std::chrono::system_clock::now();
    const auto duration = now.time_since_epoch();

    // Convert the duration to seconds with a fractional part
    const double seconds = std::chrono::duration<double>(duration).count();

    return seconds;
}

long get_current_timestamp_micro() {
    return static_cast<long>(get_current_timestamp() * 1000 * 1000);
}

long get_current_timestamp_milli() {
    return static_cast<long>(get_current_timestamp() * 1000);
}

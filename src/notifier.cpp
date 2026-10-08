#include "notifier.h"

#include <stdexcept>

Notifier::Notifier(const NotifierConfig& config) : _config(config) {}

bool Notifier::notify() {
    std::unique_lock lock(_mutex);

    const auto now = std::chrono::steady_clock::now();

    // Throttle: a new notification is only allowed after min_wait_micro has
    // elapsed since the previous one.
    if (_config.min_wait_micro.has_value() && _has_notified) {
        const auto elapsed = now - _last_notify;
        if (elapsed < std::chrono::microseconds(*_config.min_wait_micro)) {
            return false;
        }
    }

    _last_notify = now;
    _has_notified = true;
    ++_generation;

    // Unlock before waking waiters so they do not block on the mutex again.
    lock.unlock();
    _cv.notify_all();
    return true;
}

void Notifier::wait() {
    std::unique_lock lock(_mutex);

    const std::uint64_t generation = _generation;
    const auto changed = [this, generation] { return _generation != generation; };

    if (_config.max_wait_micro.has_value()) {
        // Bounded wait: return on a notification or after max_wait_micro.
        _cv.wait_for(lock, std::chrono::microseconds(*_config.max_wait_micro), changed);
    } else {
        // Unbounded wait: return only on a notification.
        _cv.wait(lock, changed);
    }
}

void Notifier::wait(const int rel_time_micro) {
    // Only min_wait_micro is enforced here: the requested timeout must not be
    // smaller than the configured lower bound.
    if (_config.min_wait_micro.has_value() && rel_time_micro < *_config.min_wait_micro) {
        throw std::invalid_argument("Notifier::wait(rel_time_micro): rel_time_micro is smaller than min_wait_micro");
    }

    std::unique_lock lock(_mutex);

    const std::uint64_t generation = _generation;
    const auto changed = [this, generation] { return _generation != generation; };

    // Explicit timeout for this call; overrides max_wait_micro from the config.
    _cv.wait_for(lock, std::chrono::microseconds(rel_time_micro), changed);
}

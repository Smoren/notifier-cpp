#pragma once

// Runs an illustrative example of Notifier usage:
// producer/consumer wake-up, min_wait_micro throttling, max_wait_micro
// watchdog timeout and the explicit wait(rel_time_micro) overload.
void run_app();

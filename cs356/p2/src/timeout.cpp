#include "timeout.h"

#include <thread>
#include <memory>

#include "logging.h"

using namespace std;

namespace util {
SimpleTimeout::SimpleTimeout(chrono::nanoseconds period)
    : period_(period),
      exists_(false) {}

SimpleTimeout::~SimpleTimeout() {
  if (exists_) {
    timeout_->Cancel();
  }
}

bool SimpleTimeout::Start(const function<void(void)> &callback) {
  // Only create new timeout if an active one doesn't exist
  if (!exists_) {
    timeout_ = new InternalTimeout([callback, this]() {
          // When callback fires, mark as going out of existence
          exists_ = false;
          callback();
        }, period_);

    return exists_ = true;
  }

  return false;
}

bool SimpleTimeout::Cancel() {
  if (exists_) {
    timeout_->Cancel();
    exists_ = false;
    return true;
  }

  return false;
}

bool SimpleTimeout::IsActive() const {
  return exists_;
}

SimpleTimeout::InternalTimeout::InternalTimeout(
    const function<void(void)> &callback,
    chrono::nanoseconds period)
    : active_(true) {
  // Spawn a thread capturing the user's callback, the period, and this.
  thread([callback, period, this]() {
        DEBUG << "Timing Thread Spawned.";
        // A unique ptr to this, guarenteeing that this thread will always take
        // ownership of this, and delete it when done.
        unique_ptr<InternalTimeout> this_deleter(this);

        // Sleep for timeout.
        this_thread::sleep_for(period);

        // Execute callback if active.
        if (active_)
          callback();

        DEBUG << "Timing Thread Destroyed.";
      }).detach();  // Detach the thread.
}
}  // namespace util

#ifndef UTIL_TIMEOUT_H_
#define UTIL_TIMEOUT_H_

#include <atomic>
#include <chrono>
#include <functional>

/// Namespace for all utility classes.
namespace util {
/// Interface for General Timeouts
class Timeout {
 public:
  // Disallow Copy & Assign
  Timeout(const Timeout &) = delete;
  Timeout &operator=(const Timeout &) = delete;
  virtual ~Timeout() = default;

  /// Starts a timeout bound to this callback.
  ///
  /// @param callback to be called upon timeout.
  /// 
  /// @returns true if a timeout was able to be started.
  virtual bool Start(const std::function<void(void)> &callback) = 0;

  /// Cancels the current timeout.
  ///
  /// @returns true if a timeout was able to be canceled.
  virtual bool Cancel() = 0;

  /// Checks whether there is an active timeout.
  ///
  /// @returns true of there is an active timeout
  virtual bool IsActive() const = 0;

 protected:
  Timeout() = default;
};

/// Simple thread based implementation of a timeout. Not incredibly precise,
/// but effective.
class SimpleTimeout : public Timeout {
 public:
  /// Create a timeout with a specified period.
  ///
  /// @param period length of timeout in nanoseconds.
  SimpleTimeout(std::chrono::nanoseconds period = std::chrono::seconds(1));
  // Disallow Copy & Assign
  SimpleTimeout(const SimpleTimeout &) = delete;
  SimpleTimeout &operator=(const SimpleTimeout &) = delete;
  ~SimpleTimeout() override;

  bool Start(const std::function<void(void)> &callback) override;
  bool Cancel() override;
  bool IsActive() const override;

 private:
  /// Spawns a thread that waits a specified timeout interval before calling a
  /// callback. This class deletes itself and MUST be allocated using single new
  /// allocation.
  class InternalTimeout {
   public:
    /// Create a timeout with a specified period. MUST be created with new 
    /// operator. DO NOT create an InternalTimeout object on the stack.
    ///
    /// @param callback the function to call after the timeout period.
    /// @param period length of timeout in nanoseconds.
    InternalTimeout(const std::function<void(void)> &callback,
        std::chrono::nanoseconds period);
    // Disallow Copy & Assign
    InternalTimeout(const InternalTimeout &) = delete;
    InternalTimeout &operator=(const InternalTimeout &) = delete;
    
    /// Cancels the timeout. The callback will not be called after the timeout
    /// period.
    void Cancel() { active_ = false; }

    /// Checks whether the callback is set to be called or not.
    ///
    /// @returns true if the timeout has not been canceled, false otherwise.
    bool IsActive() const { return active_; }

   private:
    std::atomic<bool> active_;
  };

  std::chrono::nanoseconds period_;
  InternalTimeout *timeout_;
  std::atomic<bool> exists_;
};
}  // namespace util

#endif  // UTIL_TIMEOUT_H_

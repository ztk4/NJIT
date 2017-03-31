#ifndef UTIL_THREAD_POOL_H_
#define UTIL_THREAD_POOL_H_

#include <atomic>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

namespace util {
/// A class for managing multiple threads.
/// Allows for simplistic multi-threading.
class ThreadPool {
 public:
  /// Constructs a Thread Pool with a specified maximum number of theads.
  /// 
  /// @param max_threads the maximum number of threads to dispatch at a given
  ///                    time.
  explicit ThreadPool(int max_threads) 
    : max_(max_threads),
      active_threads_(0) {}
  // Disallow Copy and Assign
  ThreadPool(const ThreadPool &) = delete;
  ThreadPool &operator=(const ThreadPool &) = delete;
  ~ThreadPool();

  /// Maps given function onto the given range. Spawns one thread per
  /// invocation. Blocks on completion of all mappings.
  /// 
  /// @param func a callable object that takes a single argument of the type
  ///             populating the range specified by begin and end.
  /// @param begin an output iterator specifying the beginning of a range.
  /// @param end an output iterator specifying the end of a range (exclusive).
  template <class T>
  void Map(const std::function<void(const typename T::value_type &arg)> &func,
      const T begin, const T end) {
    MapAsync(func, begin, end);
    JoinAll();
  }
  /// Behaves exactly like Map<T>, but does not block on completion. JoinAll
  /// can be called afterwards to synchronize tasks with the calling thread.
  template <class T>
  void MapAsync(
      const std::function<void(const typename T::value_type &arg)> &func,
      const T begin, const T end) {
    {  // Scope for lock
      std::lock_guard<std::mutex> queued_tasks_mutex_lock(queued_tasks_mutex_);

      for (T it = begin; it != end; ++it) {
        typename T::value_type val = *it;
        queued_tasks_.push([val, func]() {
              func(val);
            });
      }
    }

    // Try to flush if possible.
    Flush();
  }

  /// Executes the given function, passing in a thread id from 0 to times-1.
  /// Blocks on completion of all times executions.
  ///
  /// @param func the task to be executed.
  /// @param times the number of times to execute the task.
  void Execute(const std::function<void(int thread_num)> &func, int times);
  /// Behaves exactly like Execute(const function<void(int thread_num)> &, int)
  /// but does not block on completion. JoinAll can be called afterward to
  /// synchronize tasks with the calling thread.
  void ExecuteAsync(const std::function<void(int thread_num)> &func, int times);

  /// Executes the given function, passing no parameters. Executes the function
  /// times times, blocking on completion.
  /// 
  /// @param func the task to be executed.
  /// @param times the number of times to execute the task.
  void Execute(const std::function<void(void)> &func, int times);
  /// Behaves exactly like Execute(const function<void(void)> &, int)
  /// but does not block on completion. JoinAll can be called afterward to
  /// synchronize tasks with the calling thread.
  void ExecuteAsync(const std::function<void(void)> &func, int times);

  /// Blocks on completion of all dispatched threads, then returns.
  void JoinAll() const;

 private:
  /// Deques a single task and dispatches a thread to execute it. Does not
  /// contain logic for enforcing thread limit, but does enforce cleanup.
  /// 
  /// @returns true if a thread was dispatched.
  bool DequeueTask();
  /// Flushes tasks from queue until thread limit is reached.
  void Flush();
  /// Callback to be called by a dispatched thread after finished it's task. 
  /// Allows cleanup of resources, and it's use is enforced by DispatchThread.
  void OnThreadFinished();

  /// Maximum number of threads to be dispatched at once.
  const int max_;
  /// Queue of tasks waiting to be dispatched to a thread.
  std::queue<std::function<void(void)>> queued_tasks_;
  /// Mutex for guarding the task queue.
  mutable std::mutex queued_tasks_mutex_;  // GUARDS queued_tasks_
  /// Number of active thread managed by this pool.
  std::atomic<int> active_threads_;
};
}  // namespace util

#endif  // UTIL_THREAD_POOL_H_

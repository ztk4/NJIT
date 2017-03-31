#include "thread_pool.h"

#include "logging.h"

using namespace std;

namespace util {
ThreadPool::~ThreadPool() {
  // Don't destroy self unless all threads are joined. Threads capture
  // references to this, and so must not be active after this destructor.
  JoinAll();
}

void ThreadPool::Execute(const function<void(int thread_num)> &func,
    int times) {
  ExecuteAsync(func, times);
  JoinAll();
}

void ThreadPool::ExecuteAsync(const function<void(int thread_num)> &func,
    int times) {
  {  // Scope for lock
    lock_guard<mutex> queued_tasks_mutex_lock(queued_tasks_mutex_);

    for (int i = 0; i < times; ++i) {
      queued_tasks_.push([i, func]() {
          func(i);
        });
    }
  }

  Flush();
}

void ThreadPool::Execute(const function<void(void)> &func, int times) {
  ExecuteAsync(func, times);
  JoinAll();
}

void ThreadPool::ExecuteAsync(const function<void(void)> &func, int times) {
  {  // Scope for lock
    lock_guard<mutex> queued_tasks_mutex_lock(queued_tasks_mutex_);

    for (int i = 0; i < times; ++i)
      queued_tasks_.push(func);
  }

  Flush();
}

void ThreadPool::JoinAll() const {
  // Wait for all threads to finish.
  while (active_threads_)
    this_thread::yield();
}

bool ThreadPool::DequeueTask() {
  lock_guard<mutex> queued_tasks_mutex_lock(queued_tasks_mutex_);

  // Execute task if there is one.
  if (!queued_tasks_.empty()) {
    DEBUG << "Spawning new Task Thread.";

    function<void(void)> task = queued_tasks_.front();
    thread([this, task]() {
        task();
        OnThreadFinished();
      }).detach();
    queued_tasks_.pop();

    return true;
  }

  return false;
}

void ThreadPool::Flush() {
  while (active_threads_ < max_) {
    if (!DequeueTask())
      break;

    ++active_threads_;
  }
}

void ThreadPool::OnThreadFinished() {
  DEBUG << "Killing this Task Thread.";

  // Try executing another task since this one is done.
  if (!DequeueTask())
    --active_threads_;  // Otherwise decrement thread counter.
}
}  // namespace util

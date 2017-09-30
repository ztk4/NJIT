// Zachary Kaplan [3570]
// CS435-001
// MP 1
// 9/26/17

#ifndef MP1_PRIORITY_QUEUE_H_
#define MP1_PRIORITY_QUEUE_H_

#include <functional>
#include <initializer_list>
#include <utility>
#include <vector>

#include "utils_3570.h"

namespace mp1 {
// Priority Queue implementation for arbitrary type T.
// T is element type, Comp::operator() implements comparisons on T.
template <typename T, typename Comp=std::less<T>>
class PriorityQueue {
 public:
  // CONSTRUCTORS
  // Default ctor, constructs an empty queue.
  PriorityQueue() = default;
  // Range ctor, constructs a queue from a range of values.
  template <typename It>
  PriorityQueue(It begin, It end) : heap_(begin, end) {
    Heapify();
  }
  // Initializer List ctor, construts a queue from an initializer list.
  PriorityQueue(std::initializer_list<T> il) : heap_(il) {
    Heapify();
  }

  // Default copy/move ctor and assignment
  PriorityQueue(const PriorityQueue &) = default;
  PriorityQueue &operator=(const PriorityQueue &) = default;
  PriorityQueue(PriorityQueue &&) = default;
  PriorityQueue &operator=(PriorityQueue &&) = default;

  // PUBLIC INTERFACE
  // Inserts an element into the queue.
  void Push(const T &t) {
    heap_.push_back(t);
    SiftUp(heap_.size() - 1);
  }

  // Removes the highest priority element from the queue and returns it.
  T Pop() {
    CHECK(!empty()) << "Can't pop from an empty priority queue";

    // Move out best value.
    T val = std::move(heap_.front());
    // Swap a leaf to front, remove last elt, and sift down.
    heap_.front() = std::move(heap_.back());
    heap_.pop_back();
    SiftDown(0);

    return val;  // Move would block copy elision.
  }

  // Slightly optimized function for pushing an element while popping the
  // highest priority element. Prefer this method for a push immediately
  // followed by a pop.
  T PushPop(const T &t) {
    // Return passed value if the queue is empty, or if it is higher priority
    // than the element at the front of the queue.
    if (empty() || comp_(t, heap_.front())) {
      return t;
    }

    T val = std::move(heap_.front());
    heap_.front() = t;
    SiftDown(0);

    return val;  // Move would block copy elision.
  }

  // Returns a reference to the next element to be popped.
  const T &Peek() const {
    CHECK(!empty()) << "Can't peek an empty priortiy queue";

    return heap_.front();
  }

  // Returns current size.
  size_t size() const { return heap_.size(); }
  // Returns if empty.
  bool empty() const { return heap_.empty(); }

 private:
  // Enforces the binary-heap invariant on heap_, assuming it's unsorted.
  void Heapify() {
    // All nodes > (size - 2) / 2 are leaves of the binary-heap, and all leaves
    // are valid binary-heap's by themselves.
    // Therefore, we start the siftdown at the first node with a child:
    //   floor( (size - 2) / 2 )
    for (int node = (heap_.size() - 2) / 2; node >= 0; --node) {
      SiftDown(node);
    }
  }

  // Enforces the binary-heap invariant from heap_[node] on.
  // NB: The binary-heap invariant already holds from heap_[node+1] on.
  void SiftDown(int node) {
    // Store the value we are sifting down, this allows slightly more efficient
    // half swaps while sifting.
    T val = std::move(heap_[node]); 
    int parent = node;
    bool done = false;
    while (!done) {
      // Child indexes
      size_t child1 = 2*parent + 1, child2 = child1 + 1;
      // Reference to highest priority among val, and both children.
      const T *best = &val;  // Assume val is best.
      int best_id = parent;  // Assume index of best is parent.

      if (child1 < heap_.size() && comp_(heap_[child1], *best)) {
        // Child1 is better than our current best.
        best = &heap_[child1];
        best_id = child1;
      }

      if (child2 < heap_.size() && comp_(heap_[child2], *best)) {
        // Child2 is better than our current best.
        best = &heap_[child2];
        best_id = child2;
      }

      heap_[parent] = std::move(*best);  // Put best into parent position.
      
      if (best_id == parent) {
        // Both children hold the heap invariant, sift down is done.
        done = true;
      } else {
        // Need to continue to sift down on child we swapped from.
        parent = best_id;
      }
    }
  }

  // Enforces the binary-heap invariant from 0 to node in heap_.
  // NB: The binary-heap invariant already holds from 0 to node - 1 in heap_.
  void SiftUp(int node) {
    // Store the value we are sifting up, this allows slightly more efficient
    // half swaps while sifting.
    T val = std::move(heap_[node]);
    int child = node;
    while (child > 0) {
      int parent = (child - 1) / 2;
      if (comp_(val, heap_[parent])) {
        // Value we are sifting up is higher priority than parent.
        heap_[child] = std::move(heap_[parent]);
        child = parent;  // Continue to sift up from parent.
      } else {
        // Done sifting up, exit loop to finish half swap.
        break;
      }
    }

    heap_[child] = std::move(val);
  }

  // Instance of the comparator.
  Comp comp_;
  // Resizable array for storage.
  // Maintains a standard binary-heap invariant:
  //   comp_(heap[2*i + 1], heap[i]) == false
  //   comp_(heap[2*i + 2], heap[i]) == false 
  std::vector<T> heap_;
};
}  // namespace mp1

#endif  // MP1_PRIORITY_QUEUE_H_

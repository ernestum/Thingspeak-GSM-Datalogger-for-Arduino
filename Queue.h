#ifndef QUEUE_H
#define QUEUE_H

/// A simple fixed-length queue implementation to manage unsent sensor readings.
template <int capacity, typename T>
class Queue {
 public:
  Queue() : entry(0), exit(0), size(0) {}

  bool isEmpty() { return size == 0; }

  bool isFull() { return size == capacity; }

  /// Puts a new element into the queue.
  /// If the queue is already full, the first element in the queue is
  /// overridden.
  void enqueue(const T e) {
    if (isFull())
      exit = next(exit);
    else
      ++size;
    array[entry] = e;
    entry = next(entry);
  }

  /// Removes the first element of the queue and returns it.
  /// If the queue is empty, the last dequeued element is returned.
  T dequeue() {
    if (!isEmpty()) {
      exit = next(exit);
      --size;
    }
    return array[prev(exit)];
  }

  /// Returns the first element of the queue without removing it
  T peek() { return array[exit]; }

  /// Stores the amout of elements in the queue
  uint8_t size;

  /// Here is the actual content of the queue stored.
  T array[capacity];

 private:
  uint8_t entry, exit;

  /// Return the index of the element that comes after index i
  inline uint8_t next(uint8_t i) { return (i + 1) % capacity; }

  /// Return the index of the element that comes before index i
  inline uint8_t prev(uint8_t i) { return i == 0 ? capacity - 1 : i - 1; }
};

#endif // QUEUE_H

#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <vector>
#include <atomic>
#include "asyncLogger.h"

class TaskQueue {
public:
    // Delete copy constructor and assignment operator
    TaskQueue(const TaskQueue&) = delete;
    TaskQueue& operator=(const TaskQueue&) = delete;
    ~TaskQueue() { stop(); };

    // Static method to get singleton instance
    static TaskQueue& getInstance() {
        static TaskQueue instance;
        return instance;
    }

    void enqueue(std::function<void()> task) {
        if (!stop_) {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            tasks_.push(task);
        }
        condition_.notify_one();
    }

    std::function<void()> dequeue() {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        condition_.wait(lock, [this] { return !tasks_.empty() || stop_; });
        if (stop_ && tasks_.empty()) {
            return nullptr;
        }
        std::function<void()> task = tasks_.front();
        tasks_.pop();
        return task;
    }

    void worker_thread() {
        while (!stop_) {
            std::function<void()> task = dequeue();
            if (task) {
                task();
            }
        }
    }

    void start(int num_threads) {
        std::cout << "call start" << std::endl;
        std::call_once(start_once_flag_, [this, num_threads]() {
            std::cout << "actual start" << std::endl;
            for (int i = 0; i < num_threads; ++i) {
                workers_.emplace_back(&TaskQueue::worker_thread, this);
            }
            });
    }

    void stop() {
        std::cout << "call stop" << std::endl;
        std::call_once(stop_once_flag_, [this]() {
            std::cout << "actual stop" << std::endl;
            stop_ = true;
            condition_.notify_all();
            for (auto& worker : workers_) {
                worker.join();
            }
            {
                std::unique_lock<std::mutex> lock(queue_mutex_);
                while (!tasks_.empty()) {
                    tasks_.pop();
                }
            }
            });
    }

private:
    TaskQueue() = default;

    std::queue<std::function<void()>> tasks_;
    std::mutex queue_mutex_;
    std::condition_variable condition_;
    std::vector<std::thread> workers_;
    std::atomic<bool> stop_;
    std::once_flag stop_once_flag_;
    std::once_flag start_once_flag_;
};

// Client thread function
void client_thread(int id) {
    // Get TaskQueue singleton
    TaskQueue& queue = TaskQueue::getInstance();
    queue.start(4);

    // Add some tasks
    for (int i = 0; i < 3; ++i) {
        queue.enqueue([id, i] {
            SimpleAsyncSingletonLogger& logger = SimpleAsyncSingletonLogger::getInstance();
            std::string message = "Client " + std::to_string(id) +
                " - Task " + std::to_string(i) +
                " executed by thread: " +
                std::to_string(std::hash<std::thread::id>{}(std::this_thread::get_id()));
            logger.info(message);
            });
    }
}

int main() {
    TaskQueue& task_queue = TaskQueue::getInstance();
    task_queue.start(4);

    SimpleAsyncSingletonLogger& logger = SimpleAsyncSingletonLogger::getInstance();
    logger.setLogLevel(LogLevel::Debug);

    // Create multiple client threads
    std::vector<std::thread> clients;
    for (int i = 0; i < 3; ++i) {
        clients.emplace_back(client_thread, i);
    }

    // Wait for all client threads to complete
    for (auto& client : clients) {
        client.join();
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));
    task_queue.stop();
    return 0;
}
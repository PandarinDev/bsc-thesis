#pragma once

#include <deque>
#include <atomic>
#include <thread>
#include <vector>
#include <future>
#include <mutex>
#include <utility>
#include <functional>
#include <condition_variable>

#include <iostream>

namespace inf::utils {

    template<typename ResultType>
    struct ThreadPool {

        ThreadPool(std::size_t num_workers) : stop(false) {
            for (std::size_t i = 0; i < num_workers; ++i) {
                workers.emplace_back([&]() {
                    while (true) {
                        std::unique_lock job_queue_lock(job_queue_mutex);
                        job_queue_cv.wait(job_queue_lock, [&]() { return stop || !job_queue.empty(); });
                        if (stop) {
                            return;
                        }
                        auto entry = std::move(job_queue.front());
                        job_queue.pop_front();
                        job_queue_lock.unlock();
                        auto& promise = entry.first;
                        auto& job = entry.second;
                        promise.set_value(job());
                    }
                });
            }
        }

        ~ThreadPool() {
            stop = true;
            job_queue_cv.notify_all();
            for (auto& worker : workers) {
                worker.join();
            }
        }

        std::future<ResultType> add_job(std::function<ResultType()> job) {
            std::future<ResultType> future;
            {
                std::lock_guard<std::mutex> job_queue_lock(job_queue_mutex);
                job_queue.push_back(std::make_pair(std::promise<ResultType>(), job));
                future = job_queue.back().first.get_future();
            }
            job_queue_cv.notify_one();
            return future;
        }

    private:

        std::atomic_bool stop;
        std::deque<std::pair<std::promise<ResultType>, std::function<ResultType()>>> job_queue;
        std::condition_variable job_queue_cv;
        std::mutex job_queue_mutex;
        std::vector<std::thread> workers;

    };

}
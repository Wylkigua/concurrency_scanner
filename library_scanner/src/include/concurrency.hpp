#pragma once

#include <atomic>
#include <algorithm>
#include <mutex>
#include <queue>
#include <optional>
#include <thread>
#include <future>
#include <algorithm>
#include <utility>


namespace thread_pool {
  /**
   * @brief Очередь задач для пула потоков.
   *
   * Хранит задачи в виде std::packaged_task<void()>
   */
  struct Queue_task {
    using task_type = std::packaged_task<void()>;

    void push(task_type task);
    std::optional<task_type> try_pop();

    void push_notify(task_type task);
    std::optional<Queue_task::task_type> wait_pop();

    void notify_finish_work() noexcept;

    Queue_task() noexcept = default;

    ~Queue_task();

    private:
    std::mutex mtx;
    std::condition_variable cond_var;
    std::atomic_bool done{false};
    std::queue<task_type> queue_tasks;
  };

  template<typename T>
  concept Task = requires(T t) {
    {t.get_future()};
  };

  template<Task Task>
  struct Pool {
    using future_type = decltype(std::declval<Task>().get_future());  ///< определение future задачи
    using worker_type = void (Pool::*)();  /// < указатель для функций worker, worker_wait


    ~Pool() {
      done.store(true);
    }
    /**
     * @brief Добавляет задачу в очередь (без уведомления).
     * @param task Задача.
     * @return future, связанный с задачей.
     */
    future_type add_task(Task&& task) {
      auto f = task.get_future();
      worker_tasks.push(std::packaged_task<void()>(
        [t = std::move(task)]() mutable { t(); }
      ));
      return f;
    }

    /**
     * @brief Добавляет задачу в очередь и уведомляет рабочие потоки.
     * @param task Задача.
     * @return future, связанный с задачей.
     */
    future_type add_task_notify(Task&& task) {
      auto f = task.get_future();
      worker_tasks.push_notify(std::packaged_task<void()>(
          [t = std::move(task)]() mutable { t(); }
      ));
      return f;
    }

    /**
     * @brief Останавливает выполнение потоков (сигнал завершения).
     */
    void stop_threads() noexcept {
      worker_tasks.notify_finish_work();
    }

    /**
     * @brief Запускает пул потоков с блокирующим ожиданием задач.
     * @param threads_count_hint Подсказка по количеству потоков (default = 0).
     * @details если threads_count_hint = 0, то количество потоков определяется
     * вызовом std::thread::hardware_concurrency(), если threads_count_hint превышает
     * поддерживаемое платформой количество потоков, так же устанавливается std::thread::hardware_concurrency()
     */
    void run_pool_wait(unsigned threads_count_hint = 0) {
      start(&Pool::worker_wait, threads_count_hint);
    }
    
    /**
     * @brief Запускает пул потоков в режиме активного ожидания.
     * @param threads_count_hint Подсказка по количеству потоков (0 = авто).
     * @details если threads_count_hint = 0, то количество потоков определяется
     * вызовом std::thread::hardware_concurrency(), если threads_count_hint превышает
     * поддерживаемое платформой количество потоков, так же устанавливается std::thread::hardware_concurrency()
     */
    void run_pool(unsigned threads_count_hint = 0) {
      start(&Pool::worker, threads_count_hint);
    }

    private:
    /**
     * @brief Инициализирует потоки.
     * @param func Указатель на метод (worker или worker_wait).
     * @param threads_count_hint Подсказка по количеству потоков.
     */
    void start(worker_type func, unsigned threads_count_hint = 0) {
      const int hardware_threads_count = std::thread::hardware_concurrency();
      const int min_threads_count{1};
      if (!threads_count_hint) {
        threads_count_hint = std::max(hardware_threads_count, min_threads_count);
      } else {
        threads_count_hint = std::min({hardware_threads_count, static_cast<int>(threads_count_hint), min_threads_count});
      }
      pool.resize(threads_count_hint);
      pool.shrink_to_fit();
      for (auto& t : pool) {
        t = std::jthread{func, this};

      }
    }

    /**
     * @brief Выполнение потоков (активное ожидание).
     *
     * Поток проверяет наличие задач и выполняет их.
     * Если задач нет, отдаёт управление ОС (yield).
     */
    void worker() {
      while (!done) {
        if (auto task = worker_tasks.try_pop()) {
          (*task)();
        } else {
          std::this_thread::yield();
        }
      }
    }
    
    /**
     * @brief Выполнение потоков (блокирующее ожидание).
     *
     * Поток ждёт уведомления о добавлении задач и выполняет их.
     * Завершается, если пришло уведомлении о завершнии.
     */
    void worker_wait() {
      while (!done) {
        auto task = worker_tasks.wait_pop();
        if (!task.has_value()) break;
        (*task)();
      }
    }
    Queue_task worker_tasks{};
    std::vector<std::jthread> pool;
    std::atomic_bool done{false};
  };
}

#include "include/concurrency.hpp"

namespace thread_pool {

  /**
   * @brief Добавляет задачу в очередь без уведомления рабочих потоков.
   * @param task Задача для выполнения.
   */
  void Queue_task::push(task_type task) {
    std::scoped_lock lock(mtx);
    queue_tasks.push(std::move(task));
  }

  /**
   * @brief Извлекает задачу из очереди без ожидания либо std::nullopt.
   * @return Задача, если очередь не пуста, иначе std::nullopt.
   */
  std::optional<Queue_task::task_type> Queue_task::try_pop() {
    std::scoped_lock lock(mtx);
    if (queue_tasks.empty()) return {};
    task_type task(std::move(queue_tasks.front()));
    queue_tasks.pop();
    return task;
  }
  
  /**
   * @brief Добавляет задачу в очередь и уведомляет рабочие потоки.
   * @param task Задача для выполнения.
   */
  void Queue_task::push_notify(task_type task) {
    {
      push(std::move(task));
    }
    cond_var.notify_all();
  }
  
  /**
   * @brief Извлекает задачу из очереди, ожидая если очередь пуста.
   * Поток блокируется, пока не появится задача или не придёт сигнал завершения.
   * @return Задача или std::nullopt, если работа завершена.
   */
  std::optional<Queue_task::task_type> Queue_task::wait_pop() {
    std::unique_lock uniq_lock(mtx);
    cond_var.wait(uniq_lock, [this] {
      return !queue_tasks.empty() || done;
    });
    if (done) return {};
    task_type task(std::move(queue_tasks.front()));
    queue_tasks.pop();
    return task;
  }
  
  /**
   * @brief Уведомляет все ожидающие потоки о завершении работы.
   */
  void Queue_task::notify_finish_work() noexcept {
    done.store(true);
    cond_var.notify_all();
  }

  Queue_task::~Queue_task() {
    notify_finish_work();
  }
}

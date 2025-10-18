#pragma once

#include <map>
#include <string>

#include "concurrency.hpp"
#include "utils.hpp"
#include "md_hash.hpp"


namespace scanner_core {

  /**
  * @brief Статистика работы сканера.
  */
  struct Statistics {
    unsigned total_files{};      
    unsigned failed_files{};     
    unsigned malicious_files{};  
  };

  /**
   * @typedef hash_pool_t
   * @brief Пул потоков для задач хеширования.
   * Использует thread_pool::Pool с задачами типа md_hash::Hash::task_type.
   */
  using hash_pool_t = thread_pool::Pool<md_hash::Hash::task_type>;
  
  /**
  * @typedef logging_pool_t
  * @brief Пул потоков для задач логирования.
  * Использует thread_pool::Pool с задачами типа utils::Logger::task_type.
  */
  using logging_pool_t = thread_pool::Pool<utils::Logger::task_type>;

  /**
  * @brief Основной класс сканера файловой системы.
  *
  * Scanner выполняет две задачи:
  * - Хеширование всех файлов в указанной директории.
  * - Сравнение полученных хэшей с базой (CSV-файл) и логирование
  *   найденных совпадений (вредоносных файлов).
  */
  struct Scanner {
    void files_hashing(const std::string& directory_path);
    void files_search(const std::string& csv_file, const std::string& log_file);
    Statistics get_statistic() const noexcept;
    
    private:
    unsigned count_total_files{};  ///< Общее количество обработанных файлов.
    unsigned count_failed_files{};  ///< Количество файлов, при обработке которых произошли ошибки.
    unsigned count_malicious_files{};  ///< Количество файлов, определённых как вредоносные.
    
    std::multimap<std::string, std::string> hash_files{};
  };
}

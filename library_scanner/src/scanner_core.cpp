#include <filesystem>
#include <syncstream>
#include <iostream>

#include "include/scanner_core.hpp"


namespace scanner_core {
  /**
   * @brief Выполняет хеширование файлов в указанной директории.
   * Использует пул потоков для параллельного вычисления хэшей.
   * Для каждого файла создаётся задача, возвращающая пару {хэш, путь}.
   * Результаты сохраняются в hash_files.
   * @param directory_path Путь к директории (обходится рекурсивно).
   */
  void Scanner::files_hashing(const std::string& directory_path) {
    hash_pool_t pool;
    pool.run_pool();
    std::vector<hash_pool_t::future_type> futures;
    for (const auto& dir_entry : std::filesystem::recursive_directory_iterator(directory_path)) {
      if (dir_entry.is_regular_file()) {
        auto fut = pool.add_task(md_hash::Hash::create_hashing_task(dir_entry.path().string()));
        futures.emplace_back(std::move(fut));
      }
    }
    
    count_total_files = futures.size();
    auto insert_it = hash_files.begin();
    for (auto& f: futures) {
      /* если отлавливаем исключение - увеличиваем счетчик count_failed_files */      
      try {
        auto [hash, path] = f.get();
        insert_it = hash_files.emplace_hint(insert_it, std::move(hash), std::move(path));
      } catch (...) {
        ++count_failed_files;
      }
    }
  }

  /**
   * @brief Выполняет поиск хэшей в базе (CSV) и логирование совпадений.
   * CSV-файл должен иметь формат: <hash>;<verdict> в каждой строке.
   * Для каждого найденного совпадения в log_file записывается строка:
   * <path>:<hash>:<verdict>.
   * @param csv_file CSV-файл с базой хэшей.
   * @param log_file Файл для записи логов о совпадениях.
   */
  void Scanner::files_search(const std::string& csv_file, const std::string& log_file) {
    logging_pool_t pool;
    pool.run_pool_wait(1);
    std::vector<logging_pool_t::future_type> futures;
    utils::Parse_file file(csv_file);
    while (auto p = file.parse()) {
      const auto& hash = p->first;
      const auto& verdict = p->second;
      if (auto sougth = hash_files.find(hash); sougth != hash_files.end()) {
        ++count_malicious_files;
        auto& path = sougth->second;
        auto f = pool.add_task_notify(utils::Logger::create_logging_task(log_file, path, hash, verdict));
        futures.push_back(std::move(f));
      }
    }
    for (auto& f: futures) {
      try {
        f.get();
      } catch (...) {
        std::cerr << "fail write log file" << std::endl;
      }
    }
    /* после ожидания всех задач закрываем поток */
    pool.stop_threads();
  }
  Statistics Scanner::get_statistic() const noexcept {
    return {
      .total_files = count_total_files,
      .failed_files = count_failed_files,
      .malicious_files = count_malicious_files
    };
  }
}

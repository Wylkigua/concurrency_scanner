#include "../include/scanner.hpp"
#include "include/scanner_core.hpp"

namespace scanner {
  /**
   * @brief Запускает сканирование директории с использованием базы хэшей.
   * Эта функция является точкой входа в библиотеку.  
   * Внутри используется scanner_core::Scanner, который:
   *  - обходит директорию рекурсивно и вычисляет хэши файлов;
   *  - сравнивает полученные хэши с CSV-базой (<hash>;<verdict>);
   *  - в случае совпадения пишет запись в лог.
   *
   * Формат записи в лог-файл: <path>:<hash>:<verdict>
   *
   * @param csv_file CSV-файл с базой хэшей и вердиктов.
   * @param log_file Файл, куда будут записаны найденные совпадения.
   * @param scan_directory Директория для рекурсивного сканирования.
   *
   * @return Scan_statistics Статистика сканирования:
   *  - total_files — количество обработанных файлов;
   *  - failed_files — количество файлов, которые не удалось обработать;
   *  - malicious_files — количество файлов, найденных в базе (вредоносные).
   */
  Scan_statistics directory_scanner(
    const std::string& csv_file,
    const std::string& log_file,
    const std::string& scan_directory
  )
  {
    scanner_core::Scanner scanner;
    scanner.files_hashing(scan_directory);
    scanner.files_search(csv_file, log_file);
    auto stat = scanner.get_statistic();
    return {stat.total_files, stat.failed_files, stat.malicious_files};
  }

}

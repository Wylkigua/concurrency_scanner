#include "include/utils.hpp"
#include <fstream>
#include <string>


namespace utils {
  /**
   * @brief Возвращает либо инициализирует один раз объект Logger.
   * @return Синглтон объект Logger.
   */
  Logger& Logger::instance(const std::string& file_name) noexcept {
    if (!logger) {
      std::fstream file(file_name.data(), std::ios::out | std::ios::app);
      logger.reset(new (std::nothrow) Logger(std::move(file)));
    }
    return *logger;
  }

  /**
   * @brief Создаёт задачу логирования.
   *
   * Задача добавит новую строку в указанный лог-файл.
   * Формат записи: <path>:<hash>:<verdict>\n
   * path, hash, verdict - это результат хеширования файла и его данные
   * @param file_name Имя файла лога.
   * @param path Путь к файлу.
   * @param hash Хэш содержимого файла.
   * @param verdict Результат сканирования csv файла (вредоностность).
   * @param do_flush определяет необходимость сбрасывания содержимого сразу в файл (default = false).
   * @return std::packaged_task<void()> для выполнения в пуле потоков.
   * @throws std::runtime_error если файл не удалось открыть или записать.
   */
  Logger::task_type Logger::create_logging_task(
    const std::string& file_name, const std::string& path,
    const std::string& hash, const std::string& verdict,
    bool do_flush
  ) {
    return task_type([file_name, path, hash, verdict, do_flush] {
      auto& logger = instance(file_name);
      if (!logger) {
        throw std::runtime_error("fail init log");
      }
      auto& file = logger.file;
      if (file.bad()) {
        throw std::runtime_error("fail file log: " + std::string(strerror(errno)));
      }
      std::string entry{
        std::move(path)+
        ":"+std::move(hash)+
        ":"+std::move(verdict) +
        "\n"
      };
      file.write(entry.data(),entry.size());
      if (do_flush) file.flush();
    });
  }

  /**
   * @brief Закрывает лог-файл и сбрасывает синглтон.
   * После вызова новый instance(file_name) создаст новый объект.
   * @param file_name Имя файла, связанного с текущим логгером.
   */
  void Logger::close_file(const std::string& file_name) noexcept {
    auto& logger = instance(file_name);
    auto& file = logger.file;
    if (file.is_open()) file.close();
    logger.reset_logging();
  }

  void Logger::reset_logging() noexcept {
    logger.reset();
  }
  
  /**
   * @brief Построчно читает файл.
   * Формат строки csv: <hash>;<verdict>
   *
   * @return Пару {hash, verdict}, если строка успешно прочитана.
   * std::nullopt, если достигнут конец файла или ошибка.
   */
  std::optional<std::pair<std::string, std::string>>
  Parse_file::parse() noexcept {
    if (auto entry = std::string{}; std::getline(file, entry)) {
      auto pos = entry.rfind(';');
      std::string_view hash_entry{entry.begin(),entry.begin() + pos};
      std::string_view verdict_entry{entry.begin() + pos + 1, entry.end()};
      return std::pair(
        std::string(std::move(hash_entry)),
        std::string(std::move(verdict_entry))
      );
    }
    return {};
  }

}

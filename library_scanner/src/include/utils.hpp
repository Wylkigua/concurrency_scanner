#pragma once

#include <fstream>
#include <memory>
#include <cstring>
#include <optional>
#include <future>

namespace utils {

  struct Logger {
    using task_type = std::packaged_task<void()>;

    task_type static create_logging_task(
      const std::string& file_name, const std::string& path,
      const std::string& hash, const std::string& verdict,
      bool do_flush = false
    );

    void static close_file(const std::string& file_name) noexcept;
    
    private:
    explicit operator bool() const noexcept {
      return logger != nullptr;
    }
    static Logger& instance(const std::string& file_name) noexcept;
    static void reset_logging() noexcept;
    
    Logger(std::fstream&& file) : file(std::move(file)) {}
    
    std::fstream file;
    static inline std::unique_ptr<Logger> logger;  ///< Singleton объект Logger.
  };

  struct Parse_file {
    Parse_file(const std::string_view file_name) : file(file_name.data(), std::ios::in)
    {}
    std::optional<std::pair<std::string, std::string>> parse() noexcept;
    private:
    std::fstream file;
  };

}

#pragma once

#if defined(_WIN32) || defined(_WIN64)   // Windows
  #ifdef LIB_SCANNER_EXPORTS
    #define LIB_SCANNER_API __declspec(dllexport)
  #else
    #define LIB_SCANNER_API __declspec(dllimport)
  #endif
#else                                    // Linux
  #if __GNUC__ >= 4
    #define LIB_SCANNER_API __attribute__((visibility("default")))
  #else
    #define LIB_SCANNER_API
  #endif
#endif

#include <string>

namespace scanner {

  struct Scan_statistics {
    unsigned total_files{};  ///< Общее количество обработанных файлов.
    unsigned failed_files{};  ///< Количество файлов, при обработке которых произошли ошибки.
    unsigned malicious_files{};  ///< Количество файлов, определённых как вредоносные.
  };

  LIB_SCANNER_API
  Scan_statistics directory_scanner(
    const std::string& csv_file,
    const std::string& log_file,
    const std::string& scan_directory
  );
}

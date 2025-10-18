#include "../library_scanner/include/scanner.hpp"
#include <cstdlib>
#include <iostream>
#include <string>
#include <chrono>
#include <unordered_map>
#include <variant>
#include <optional>


using arguments = std::tuple<std::string, std::string, std::string>;

std::variant<arguments, std::string>
args_parser(int argc, const char** argv);

int main(int argc, const char** argv) {
  if (argc != 7) {
    std::cout <<  "using scanner.exe --base <csv file>"
                  "--log <log file> --path <scan folder>";
    return EXIT_FAILURE;
  }
  try {
    auto parsed = args_parser(argc, argv);
    if (auto error = std::get_if<std::string>(&parsed)) {
      std::cerr << *error << std::endl;
      return EXIT_FAILURE;
    }
    auto [csv_file, log_file, folder_scan] = std::get<arguments>(parsed);

    auto start = std::chrono::steady_clock::now();

    auto statistics = scanner::directory_scanner(csv_file, log_file, folder_scan);

    auto end = std::chrono::steady_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "total count files processed: " << statistics.total_files << std::endl;
    std::cout << "count malicious files detected: " << statistics.malicious_files << std::endl;
    std::cout << "count file analysis errors: " << statistics.failed_files << std::endl;
    std::cout << "utility execution time (milliseconds): " << duration.count() << std::endl;

    return EXIT_SUCCESS;

  } catch (const std::exception& e) {
    std::cerr << "exception: " << e.what() << std::endl;
    return EXIT_FAILURE;
  } catch (...) {
    std::cerr << "unknown exception" << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_FAILURE;
}


std::variant<arguments, std::string>
args_parser(int argc, const char** argv) {
  std::unordered_map<std::string, std::string> args;
  for (int i = 1; i < argc; i+=2) {
    args.emplace(argv[i], argv[i+1]);
  }
  auto check = [&args](const std::string& argument) -> std::optional<std::string> {
    if (!args.contains(argument)) {
      return std::string("not used argument: " + argument);
    }
    return {};
  };
  if (auto err = check("--base")) return *err;
  if (auto err = check("--log")) return *err;
  if (auto err = check("--path")) return *err;
  return std::tuple(
    std::move(args["--base"]),
    std::move(args["--log"]),
    std::move(args["--path"])
  );
};

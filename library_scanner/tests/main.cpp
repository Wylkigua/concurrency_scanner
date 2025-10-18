#include "../src/include/scanner_core.hpp"
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;
using namespace md_hash;
using namespace utils;
using namespace thread_pool;
using namespace scanner_core;


static void write_file(const std::string& path, const std::string& content) {
    std::ofstream ofs(path);
    ofs << content;
}

static std::string compute_md5(const std::string& path) {
  std::ifstream file(path);
  EVP_MD_CTX* ctx = EVP_MD_CTX_new();
  const EVP_MD* md = EVP_md5();
  EVP_DigestInit_ex(ctx, md, nullptr);

  std::array<char, 1024> buffer{};
  while (file.good()) {
      file.read(buffer.data(), buffer.size());
      if (file.gcount() > 0) {
          EVP_DigestUpdate(ctx, buffer.data(), file.gcount());
      }
  }
  unsigned char result[EVP_MAX_MD_SIZE];
  unsigned int length{0};

  EVP_DigestFinal_ex(ctx, result, &length);

  EVP_MD_CTX_free(ctx);

  std::ostringstream hash;
  hash << std::hex << std::setfill('0');
  for (unsigned i = 0; i < length; i++) {
      hash << std::setw(2) << static_cast<int>(result[i]);
  }
  return hash.str();
}

TEST(Logger_test, Write_entry) {
  const std::string log_file{"test_log.txt"};
  if (fs::exists(log_file)) fs::remove(log_file);

  const std::string path{"path/file.txt"};
  const std::string hash{"ad02f2dc82b3227c0f875d48baa62237"};
  const std::string verdict{"Dropper"};

  auto task = Logger::create_logging_task(log_file, path, hash, verdict, true);
  task();
  Logger::close_file(log_file);
  std::ifstream file(log_file);

  ASSERT_TRUE(file.is_open());

  std::string line;
  std::getline(file, line);

  const std::string expected_entry{path + ":" + hash + ":" + verdict};
  EXPECT_EQ(line, expected_entry);

  file.close();
  fs::remove(log_file);
}


TEST(Parse_file_test, Parse_entry) {
  const std::string file_name{"test_parse.csv"};
  const std::string hash{"ad02f2dc82b3227c0f875d48baa62237"};
  const std::string verdict{"Dropper"};

  {
    std::ofstream file(file_name);
    file << hash << ";" << verdict << std::endl;
  }

  {
    Parse_file parser(file_name);
    auto result = parser.parse();
    auto next = parser.parse();

    EXPECT_FALSE(next.has_value());
    ASSERT_TRUE(result.has_value());

    EXPECT_EQ(result->first, hash);
    EXPECT_EQ(result->second, verdict);
  }

  fs::remove(file_name);
}

TEST(Queue_task_test, Push_try_pop) {

  Queue_task q;
  bool executed{false};
  Queue_task::task_type task([&executed] { executed = true; });
  q.push(std::move(task));

  auto t = q.try_pop();

  ASSERT_TRUE(t.has_value());

  (*t)();

  EXPECT_TRUE(executed);

  auto t2 = q.try_pop();

  EXPECT_FALSE(t2.has_value());
}

TEST(Thread_pool_hashing_test, Concurrnency_hashing_tasks) {
  const std::string hashable_file_1{"file_1.txt"};
  const std::string hashable_file_2{"file_2.txt"};

  write_file(hashable_file_1, "hello");
  write_file(hashable_file_2, "goodbye");

  hash_pool_t pool;
  pool.run_pool(2);

  auto task_1 = Hash::create_hashing_task(hashable_file_1);
  auto task_2 = Hash::create_hashing_task(hashable_file_2);

  auto future_1 = pool.add_task(std::move(task_1));
  auto future_2 = pool.add_task(std::move(task_2));

  auto [hash_1, file_name_1] = future_1.get();
  auto [hash_2, file_name_2] = future_2.get();

  EXPECT_EQ(file_name_1, hashable_file_1);
  EXPECT_EQ(file_name_2, hashable_file_2);

  EXPECT_EQ(hash_1, compute_md5(hashable_file_1));
  EXPECT_EQ(hash_2, compute_md5(hashable_file_2));
  EXPECT_NE(hash_1, hash_2);

  fs::remove(hashable_file_1);
  fs::remove(hashable_file_2);
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

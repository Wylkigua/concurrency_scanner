#include <fstream>
#include <algorithm>
#include <sstream>
#include <iomanip>

#include "include/md_hash.hpp"

namespace md_hash {
  /**
   * @brief Возвращает либо инициализирует один раз thread_local объект Hash.
   * Инициализирует OpenSSL контекст MD5.
   * @return Синглтон объект Hash.
   */
   Hash& Hash::instance() noexcept
  {
    if (!object) {
      if (auto ctx = EVP_MD_CTX_new(); ctx) {
        object.reset(new (std::nothrow) Hash(ctx, EVP_md5()));
      }
    }
    if (object) {
      EVP_MD_CTX_reset(object->context);
      EVP_DigestInit_ex(object->context,object->md, nullptr);
    }
    return *object;
  }
  /**
   * @brief Получает строковое описание последней ошибки OpenSSL.
   * @return Текст ошибки.
   */
  std::string Hash::get_md_error() const noexcept {
    unsigned long error_code = ERR_get_error();
    char error_buffer[size_error_buffer];
    ERR_error_string_n(error_code, error_buffer, sizeof(error_buffer));
    return error_buffer;
  };
  
  /**
   * @brief Создаёт задачу (packaged_task) для вычисления MD5 хеша файла.
   * @param file_name Имя файла, для которого нужно посчитать хеш.
   * @return packaged_task, возвращающий пару <строковый хеш, имя файла>.
   * @throws std::runtime_error при ошибках с файлом либо с OpenSSL.
   * 
   * Задача открывает файл, читает содержимое, обновляет контекст MD5
   * и возвращает итоговый хеш в hex-строке вместе с именем файла.
   * при ошибке файла или при ошибке OpenSSL генерируется исключение.
   */
  Hash::task_type
  Hash::create_hashing_task(const std::string& file_name) {
    return task_type(
      [file_name]()-> std::pair<std::string,std::string> {
        Hash& md = instance();
        if (!md) {
          throw std::runtime_error("fail init hash md context");
        }
        std::fstream file(file_name.data());
        if (file.fail()) {
          throw std::runtime_error("fail file open: " + std::string(strerror(errno)));
        }
        auto& context = md.object->context;
        auto& result = md.finaly_result;

        std::array<char,size_buffer_update_ctx> buffer;

        while(!file.fail()) {
          file.read(buffer.data(),buffer.size());
          if (EVP_DigestUpdate(context, buffer.data(), file.gcount()) != 1) {
            throw std::runtime_error(md.get_md_error());
          };
        }
        unsigned int length{};
        if (EVP_DigestFinal_ex(context, result.data(), &length) != 1) {
          throw std::runtime_error(md.get_md_error());
        }
        std::ostringstream hash;
        hash << std::hex << std::setfill('0');
        std::for_each_n(result.begin(), length, [&hash](unsigned char c) {
          hash << std::setw(2) << static_cast<int>(c);
        });
        return {std::move(hash.str()), file_name};
      });
  }
}

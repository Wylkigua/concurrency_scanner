#pragma once

#include <array>
#include <cerrno>
#include <cstring>
#include <memory>

#include <future>

#include <openssl/evp.h>
#include <openssl/md5.h>
#include <openssl/err.h>

namespace md_hash {

  struct Hash {
    using task_type = std::packaged_task<std::pair<std::string,std::string>()>; ///< Тип выполняемой задачи packaged_task
    task_type static create_hashing_task(const std::string& file_name);

    /**
    * @brief Деструктор. Освобождает контекст OpenSSL EVP_MD_CTX.
    */
    ~Hash() {
      EVP_MD_CTX_free(context);
    }

    private:
    EVP_MD_CTX* context{}; ///< Контекст OpenSSL MD5.
    const EVP_MD* md{};  ///< Указатель на алгоритм хеширования MD5.
    static constexpr unsigned size_error_buffer{256};  ///< Размер буфера ошибок OpenSSL.
    static constexpr unsigned size_buffer_update_ctx{4096};  ///< Размер буфера для поблочного чтения файла.
    static inline thread_local std::array<unsigned char,EVP_MAX_MD_SIZE> finaly_result; ///< Буфер для финального результата хеша (thread_local).
    static inline thread_local std::unique_ptr<Hash> object{};  ///< Singleton объект Hash (thread_local).

    Hash(EVP_MD_CTX* ctx, const EVP_MD* m)
    :context(ctx), md(m) {}

    static Hash& instance() noexcept;
    std::string get_md_error() const noexcept;

    explicit operator bool() const noexcept {
      return object != nullptr;
    }
  };
} 

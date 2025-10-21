/*
** ETIB PROJECT, 2025
** evan
** File description:
** FileAsset
*/

#pragma once

#include <cstring>
#include <fstream>
#include <memory>
#include <string>

/**
 * @namespace evan
 * @brief The evan namespace contains classes and functions for the evan
 * project.
 */
namespace evan {
/**
 * @class FileAsset
 * @brief The FileAsset class represents a file asset in the evan project.
 *
 * This class provides methods to read and write binary data to and from a file.
 */
class FileAsset {
public:
  /**
   * @enum Seek
   * @brief The Seek enum represents the different seek modes for file
   * operations.
   */
  enum class Seek {
    SET, ///> Seek from the beginning of the file
    CUR, ///> Seek from the current position in the file
    END  ///> Seek from the end of the file
  };

  /**
   * @brief Constructs a FileAsset object with its content and size.
   * @param content The content of the file.
   */
  FileAsset(const std::string &content);

  /**
   * @brief Destructs the FileAsset object.
   */
  ~FileAsset();

  /**
   * @brief Writes data to the file.
   * @param ptr The pointer to the data to write.
   * @param size The size of each element to write.
   * @param nmemb The number of elements to write.
   * @return The number of elements written.
   *
   * This method appends data at the seek position in the file.
   */
  size_t write(const void *ptr, size_t size, size_t nmemb);

  /**
   * @brief Reads data from the file.
   * @param ptr The pointer to the buffer to read data into.
   * @param size The size of each element to read.
   * @param count The number of elements to read.
   * @return The number of elements read.
   */
  size_t read(void *ptr, size_t size, size_t count);

  /**
   * @brief Reads data from the file.
   * @param str The string to read data into.
   * @param size The size of each element to read.
   * @param count The number of elements to read.
   * @return The number of elements read.
   */
  size_t read(std::string &str, size_t size, size_t count);

  /**
   * @brief Seeks to a specific position in the file.
   * @param offset The offset to seek to.
   * @param whence The seek mode (SET, CUR, END).
   * @return 0 on success, -1 on failure.
   */
  int seek(long offset, Seek whence);

  /**
   * @brief Returns the current position in the file.
   * @return The current position in the file.
   */
  size_t tell();

  [[__nodiscard__]] inline const std::string &content() const {
    return _content;
  }

protected:
  std::string _content; ///> The content of the file
  size_t _pos;          ///> The current position in the file
};
} // namespace evan

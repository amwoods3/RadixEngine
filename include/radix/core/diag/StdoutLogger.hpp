#ifndef RADIX_STDOUT_LOGGER_HPP
#define RADIX_STDOUT_LOGGER_HPP

#include <mutex>
#include <string>

#include <radix/core/diag/Logger.hpp>

namespace radix {

/** \class StdoutLogger
 * @brief Logger that outputs to an ANSI/vt-100 console
 */
class StdoutLogger : public Logger {
protected:
  std::mutex mtx;

public:
  const char* getName() const;
  void log(const std::string &message, LogLevel lvl, const std::string &tag);
};

} /* namespace radix */

#endif /* RADIX_STDOUT_LOGGER_HPP */

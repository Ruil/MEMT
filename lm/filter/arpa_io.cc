#include "lm/filter/arpa_io.hh"
#include "util/file_piece.hh"

#include <iostream>
#include <ostream>
#include <string>
#include <vector>

#include <ctype.h>
#include <errno.h>
#include <string.h>

namespace lm {

ARPAInputException::ARPAInputException(const StringPiece &message) throw() : what_("Error: ") {
  what_.append(message.data(), message.size());
}

ARPAInputException::ARPAInputException(const StringPiece &message, const StringPiece &line) throw() {
  what_ = "Error: ";
  what_.append(message.data(), message.size());
  what_ += " in line '";
  what_.append(line.data(), line.size());
  what_ += "'.";
}

ARPAOutputException::ARPAOutputException(const char *message, const std::string &file_name) throw()
  : what_(std::string(message) + " file " + file_name), file_name_(file_name) {
  if (errno) {
    char buf[1024];
    buf[0] = 0;
#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600) && ! _GNU_SOURCE
    const char *add = buf;
    if (!strerror_r(errno, buf, 1024)) {
#else
    const char *add = strerror_r(errno, buf, 1024);
    if (add) {
#endif
      what_ += " :";
      what_ += add;
    }
  }
}

// Seeking is the responsibility of the caller.
void WriteCounts(std::ostream &out, const std::vector<size_t> &number) {
  out << "\n\\data\\\n";
  for (unsigned int i = 0; i < number.size(); ++i) {
    out << "ngram " << i+1 << "=" << number[i] << '\n';
  }
  out << '\n';
}

size_t SizeNeededForCounts(const std::vector<size_t> &number) {
  std::ostringstream buf;
  WriteCounts(buf, number);
  return buf.tellp();
}

bool IsEntirelyWhiteSpace(const StringPiece &line) {
  for (size_t i = 0; i < static_cast<size_t>(line.size()); ++i) {
    if (!isspace(line.data()[i])) return false;
  }
  return true;
}

ARPAOutput::ARPAOutput(const char *name, size_t buffer_size) : file_name_(name), buffer_(new char[buffer_size]) {
  try {
    file_.exceptions(std::ostream::eofbit | std::ostream::failbit | std::ostream::badbit);
    if (!file_.rdbuf()->pubsetbuf(buffer_.get(), buffer_size)) {
      std::cerr << "Warning: could not enlarge buffer for " << name << std::endl;
      buffer_.reset();
    }
    file_.open(name, std::ios::out | std::ios::binary);
  } catch (const std::ios_base::failure &f) {
    throw ARPAOutputException("Opening", file_name_);
  }
}

void ARPAOutput::ReserveForCounts(std::streampos reserve) {
  try {
    for (std::streampos i = 0; i < reserve; i += std::streampos(1)) {
      file_ << '\n';
    }
  } catch (const std::ios_base::failure &f) {
    throw ARPAOutputException("Writing blanks to reserve space for counts to ", file_name_);
  }
}

void ARPAOutput::BeginLength(unsigned int length) {
  fast_counter_ = 0;
  try {
    file_ << '\\' << length << "-grams:" << '\n';
  } catch (const std::ios_base::failure &f) {
    throw ARPAOutputException("Writing n-gram header to ", file_name_);
  }
}

void ARPAOutput::EndLength(unsigned int length) {
  try {
    file_ << '\n';
  } catch (const std::ios_base::failure &f) {
    throw ARPAOutputException("Writing blank at end of count list to ", file_name_);
  }
  if (length > counts_.size()) {
    counts_.resize(length);
  }
  counts_[length - 1] = fast_counter_;
}

void ARPAOutput::Finish() {
  try {
    file_ << "\\end\\\n";
    file_.seekp(0);
    WriteCounts(file_, counts_);
    file_ << std::flush;
  } catch (const std::ios_base::failure &f) {
    throw ARPAOutputException("Finishing including writing counts at beginning to ", file_name_);
  }
}

} // namespace lm

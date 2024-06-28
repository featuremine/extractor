
#include "fmc/files.h"

#include <cassert>
#include <functional>
#include <string>
#include <string_view>

class parse_buf {
public:
  parse_buf(FILE *f, bool is_pipe = false) : file_(f), is_pipe(is_pipe) {}
  ~parse_buf() {
    if (file_) {
      if (is_pipe) {
        fmc_error_t *err = nullptr;
        fmc_pclose(file_, &err);
      } else
        fclose(file_);
    }
  }
  std::string_view view() const { return buf_; }
  int read_line() {
    ++line_;
    buf_.clear();
    int c;
    while ((c = fgetc(file_)) != EOF) {
      if (c == '\r') {
        try_eol();
        break;
      }
      if (c == '\n')
        break;
      buf_.push_back(c);
    }
    if (ferror(file_)) {
      return -1;
    }
    return buf_.size();
  }

  std::pair<bool, size_t> try_read_line() {
    auto file_pos = ftell(file_);
    buf_.clear();

    int c = 0;
    while ((c = fgetc(file_)) != EOF) {
      if (c == '\n')
        break;

      buf_.push_back(c);
    }

    if (c == '\n')
      return {true, buf_.size()};

    fseek(file_, file_pos, SEEK_SET);
    buf_.clear();
    return {false, 0};
  }

  FILE *file_;
  bool is_pipe;

private:
  void try_eol() {
    int c = fgetc(file_);
    if (c != '\n') {
      ungetc(c, file_);
    }
  }
  int line_ = 0;
  std::string buf_;
};

class csv_reader {
public:
  explicit csv_reader(FILE *f, bool is_pipe) : file_(f), is_pipe_(is_pipe) {}
  ~csv_reader() {
    if (file_ == nullptr)
      return;
    if (is_pipe_) {
      fmc_error_t *err = nullptr;
      fmc_pclose(file_, &err);
    } else
      fclose(file_);
  }
  std::string_view view() const {
    return std::string_view(buf_.data(), buf_.size() - 1);
  }
  int try_read_line() {
    if (!buf_.empty() && buf_[buf_.size() - 1] == '\n')
      buf_.clear();
    int c = 0;
    while ((c = fgetc(file_)) != EOF) {
      buf_.push_back(c);
      if (c == '\n')
        break;
    }
    if (ferror(file_))
      return -1;
    if (feof(file_))
      clearerr(file_);
    return c == '\n' ? 1 : 0;
  }

private:
  FILE *file_;
  bool is_pipe_;
  std::string buf_;
};

inline std::string_view::size_type parse_column(std::string_view str) {
  // @todo need to adhere to
  // https://en.wikipedia.org/wiki/Comma-separated_values
  if (!str.size())
    return 0;
  if (str[0] != '"') {
    auto pos = str.find_first_of(',');
    return pos == str.npos ? str.size() : pos;
  }
  std::string_view::size_type curr = 1;
  str = str.substr(1);
  do {
    auto pos = str.find_first_of('"');
    if (pos == str.npos)
      return str.npos;
    curr += pos + 1;
    if (pos + 1 == str.size() || str[pos + 1] == ',')
      break;
    if (str[pos + 1] != '"')
      return str.npos;
    ++curr;
    ++pos;
    str = str.substr(pos + 1);
  } while (str.size());
  return curr;
}

struct csv_column_info {
  std::string name;
  fm_type_decl_cp type;
  std::string format;
};

using csv_parser = std::function<int(std::string_view, fm_frame_t *, int)>;

static csv_parser get_column_parser(fm_type_sys_t *ts, fm_frame_t *frame,
                                    csv_column_info *info) {
  auto offset = fm_frame_field(frame, info->name.c_str());
  assert(fm_field_valid(offset));
  auto *type_parser = fm_type_io_get(ts, info->type);
  return [=](std::string_view str, fm_frame_t *f, int row) -> int {
    auto result = parse_column(str);
    if (result == std::string_view::npos)
      return -1;
    auto *first = str.data();
    auto *last = first + result;
    void *slot = fm_frame_get_ptr1(f, offset, row);
    auto *res = fm_type_io_parse(type_parser, first, last, slot);
    if (res != last)
      return -1;
    return result;
  };
}

inline std::string_view::size_type parse_char(std::string_view str, char c) {
  if (!str.empty() && str[0] == c)
    return 1;
  return 0;
}

inline std::string_view::size_type parse_comma(std::string_view str) {
  return parse_char(str, ',');
}

inline std::string_view::size_type parse_header(std::string_view str) {
  return parse_column(str);
}

inline std::string_view::size_type
skip_parser(std::string_view str, fm_frame_t * /* frame */, int /* row */) {
  auto result = parse_column(str);
  if (result == std::string_view::npos)
    return -1;
  return result;
}

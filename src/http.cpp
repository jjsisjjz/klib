#include "klib/http.h"

#include <cassert>
#include <cstddef>
#include <filesystem>
#include <string_view>

#include <curl/curl.h>
#include <fmt/compile.h>
#include <fmt/format.h>
#include <boost/algorithm/string.hpp>

#include "klib/error.h"
#include "klib/exception.h"
#include "klib/util.h"

// https://curl.se/libcurl/c/multi-single.html
// https://curl.se/libcurl/c/http2-download.html
// https://curl.se/libcurl/c/multi-post.html
namespace klib {

namespace {

void check_curl_correct(CURLcode code) {
  if (code != CURLcode::CURLE_OK) {
    throw RuntimeError(curl_easy_strerror(code));
  }
}

void check_curl_correct(CURLMcode code) {
  if (code != CURLMcode::CURLM_OK) {
    throw RuntimeError(curl_multi_strerror(code));
  }
}

std::string convert_non_ascii(const std::string &str) {
  std::string result;
  auto utf32 = utf8_to_utf32(str);

  for (auto c : utf32) {
    if (!is_ascii(c)) {
      auto utf8 = utf32_to_utf8(c);
      for (auto cc : utf8) {
        result +=
            fmt::format(FMT_COMPILE("%{:02X}"), static_cast<std::uint8_t>(cc));
      }
    } else {
      result.append(utf32_to_utf8(c));
    }
  }

  return result;
}

std::string splicing_url(
    const std::string &url,
    const std::unordered_map<std::string, std::string> &params) {
  if (std::empty(params)) {
    return url;
  }

  auto result = url + "?";
  for (const auto &[key, value] : params) {
    result.append(convert_non_ascii(key))
        .append("=")
        .append(convert_non_ascii(value))
        .append("&");
  }
  result.pop_back();

  return result;
}

class AddHeader {
 public:
  explicit AddHeader(
      CURL *curl, const std::unordered_map<std::string, std::string> &headers)
      : curl_(curl) {
    assert(curl_);

    if (std::empty(headers)) {
      return;
    }

    try {
      for (const auto &[key, value] : headers) {
        assert(!std::empty(key) && !std::empty(value));

        std::string str = key;
        str.append(": ").append(value);
        chunk_ = curl_slist_append(chunk_, str.c_str());
      }

      check_curl_correct(curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, chunk_));
    } catch (...) {
      curl_slist_free_all(chunk_);
      throw;
    }
  }

  ~AddHeader() {
    curl_slist_free_all(chunk_);

    try {
      check_curl_correct(curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, nullptr));
    } catch (...) {
      error("Error restoring the default header");
    }
  }

 private:
  CURL *curl_ = nullptr;
  curl_slist *chunk_ = nullptr;
};

class AddForm {
 public:
  explicit AddForm(CURL *curl,
                   const std::unordered_map<std::string, std::string> &data,
                   const std::unordered_map<std::string, std::string> &file)
      : curl_(curl) {
    assert(curl_);

    if (std::empty(data)) {
      return;
    }

    try {
      form_ = curl_mime_init(curl_);

      for (const auto &[key, value] : data) {
        assert(!std::empty(key) && !std::empty(value));

        auto field = curl_mime_addpart(form_);
        curl_mime_name(field, key.c_str());
        curl_mime_data(field, value.c_str(), CURL_ZERO_TERMINATED);
      }

      for (const auto &[file_name, path] : file) {
        assert(!std::empty(file_name) && !std::empty(path));

        if (!std::filesystem::is_regular_file(path)) {
          throw RuntimeError("file: {} not exist", path);
        }

        auto field = curl_mime_addpart(form_);
        curl_mime_name(field, file_name.c_str());
        curl_mime_filedata(field, path.c_str());
      }

      check_curl_correct(curl_easy_setopt(curl_, CURLOPT_MIMEPOST, form_));
    } catch (...) {
      curl_mime_free(form_);
      throw;
    }
  }

  ~AddForm() {
    curl_mime_free(form_);

    try {
      check_curl_correct(curl_easy_setopt(curl_, CURLOPT_MIMEPOST, nullptr));
    } catch (...) {
      error("Error restoring the default header");
    }
  }

 private:
  CURL *curl_ = nullptr;
  curl_mime *form_ = nullptr;
};

class Multi {
 public:
  explicit Multi(CURL *curl) : curl_(curl), multi_(curl_multi_init()) {
    assert(curl_);

    if (!multi_) {
      throw RuntimeError("create multi_handle error");
    }

    try {
      check_curl_correct(curl_multi_add_handle(multi_, curl_));
    } catch (...) {
      if (curl_multi_cleanup(multi_) != CURLMcode::CURLM_OK) {
        error("curl_multi_cleanup error");
      } else {
        throw;
      }
    }
  }

  ~Multi() {
    try {
      check_curl_correct(curl_multi_remove_handle(multi_, curl_));
      check_curl_correct(curl_multi_cleanup(multi_));
    } catch (...) {
      error("Error destroying multi");
    }
  }

  [[nodiscard]] CURLM *get() const { return multi_; }

 private:
  CURL *curl_ = nullptr;
  CURLM *multi_ = nullptr;
};

}  // namespace

class Request::RequestImpl {
 public:
  RequestImpl();

  RequestImpl(const RequestImpl &) = delete;
  RequestImpl(RequestImpl &&) = delete;
  RequestImpl &operator=(const RequestImpl &) = delete;
  RequestImpl &operator=(RequestImpl &&) = delete;
  ~RequestImpl();

  void verbose(bool flag);
  void allow_redirects(bool flag);
  void use_http_1_1();
  void use_http_2();
  void set_proxy(const std::string &proxy);
  void set_no_proxy();
  void set_user_agent(const std::string &user_agent);
  void set_browser_user_agent();
  void set_curl_user_agent();
  void set_timeout(std::int64_t seconds);
  void set_connect_timeout(std::int64_t seconds);

  Response get(const std::string &url,
               const std::unordered_map<std::string, std::string> &params,
               const std::unordered_map<std::string, std::string> &header);
  Response post(const std::string &url,
                const std::unordered_map<std::string, std::string> &data,
                const std::unordered_map<std::string, std::string> &file,
                const std::unordered_map<std::string, std::string> &header);
  Response post(const std::string &url, const std::string &data,
                const std::unordered_map<std::string, std::string> &header);

 private:
  constexpr static std::string_view cookies_path = "/tmp/cookies.txt";
  void set_cookies();

  Response do_post();

  static std::size_t callback_func_std_string(void *contents, std::size_t size,
                                              std::size_t nmemb,
                                              std::string *s);

  CURL *http_handle_;
};

Request::RequestImpl::RequestImpl() {
  check_curl_correct(curl_global_init(CURL_GLOBAL_DEFAULT));

  http_handle_ = curl_easy_init();
  if (!http_handle_) {
    throw RuntimeError("curl_easy_init() error");
  }

  try {
    check_curl_correct(
        curl_easy_setopt(http_handle_, CURLOPT_SSL_VERIFYPEER, 1L));
    check_curl_correct(
        curl_easy_setopt(http_handle_, CURLOPT_SSL_VERIFYHOST, 2L));
    check_curl_correct(
        curl_easy_setopt(http_handle_, CURLOPT_CAPATH, "/etc/ssl/certs"));
    check_curl_correct(curl_easy_setopt(http_handle_, CURLOPT_CAINFO,
                                        "/etc/ssl/certs/ca-certificates.crt"));
    check_curl_correct(curl_easy_setopt(http_handle_, CURLOPT_HTTP_VERSION,
                                        CURL_HTTP_VERSION_2_0));
    check_curl_correct(
        curl_easy_setopt(http_handle_, CURLOPT_FOLLOWLOCATION, 1L));
    check_curl_correct(curl_easy_setopt(http_handle_, CURLOPT_WRITEFUNCTION,
                                        RequestImpl::callback_func_std_string));
    check_curl_correct(curl_easy_setopt(http_handle_, CURLOPT_HEADERFUNCTION,
                                        callback_func_std_string));

    check_curl_correct(curl_easy_setopt(http_handle_, CURLOPT_COOKIEJAR,
                                        RequestImpl::cookies_path.data()));
  } catch (...) {
    curl_easy_cleanup(http_handle_);
    curl_global_cleanup();
    throw;
  }
}

Request::RequestImpl::~RequestImpl() {
  curl_easy_cleanup(http_handle_);
  curl_global_cleanup();
}

void Request::RequestImpl::verbose(bool flag) {
  check_curl_correct(curl_easy_setopt(http_handle_, CURLOPT_VERBOSE, flag));
}

void Request::RequestImpl::use_http_1_1() {
  check_curl_correct(curl_easy_setopt(http_handle_, CURLOPT_HTTP_VERSION,
                                      CURL_HTTP_VERSION_1_1));
}

void Request::RequestImpl::use_http_2() {
  check_curl_correct(curl_easy_setopt(http_handle_, CURLOPT_HTTP_VERSION,
                                      CURL_HTTP_VERSION_2_0));
}

void Request::RequestImpl::allow_redirects(bool flag) {
  check_curl_correct(
      curl_easy_setopt(http_handle_, CURLOPT_FOLLOWLOCATION, flag));
}

void Request::RequestImpl::set_proxy(const std::string &proxy) {
  check_curl_correct(
      curl_easy_setopt(http_handle_, CURLOPT_PROXY, proxy.c_str()));
}

void Request::RequestImpl::set_no_proxy() {
  check_curl_correct(curl_easy_setopt(http_handle_, CURLOPT_NOPROXY, "*"));
}

void Request::RequestImpl::set_user_agent(const std::string &user_agent) {
  check_curl_correct(
      curl_easy_setopt(http_handle_, CURLOPT_USERAGENT, user_agent.c_str()));
}

void Request::RequestImpl::set_browser_user_agent() {
  // navigator.userAgent
  set_user_agent(
      "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) "
      "Chrome/93.0.4577.25 Safari/537.36 Edg/93.0.961.18");
}

void Request::RequestImpl::set_curl_user_agent() {
  set_user_agent("curl/7.78.0");
}

void Request::RequestImpl::set_timeout(std::int64_t seconds) {
  check_curl_correct(curl_easy_setopt(http_handle_, CURLOPT_TIMEOUT, seconds));
}

void Request::RequestImpl::set_connect_timeout(std::int64_t seconds) {
  check_curl_correct(
      curl_easy_setopt(http_handle_, CURLOPT_CONNECTTIMEOUT, seconds));
}

Response Request::RequestImpl::get(
    const std::string &url,
    const std::unordered_map<std::string, std::string> &params,
    const std::unordered_map<std::string, std::string> &header) {
  set_cookies();
  check_curl_correct(curl_easy_setopt(http_handle_, CURLOPT_HTTPGET, 1L));

  AddHeader add_header(http_handle_, header);

  auto complete_url = splicing_url(url, params);
  check_curl_correct(
      curl_easy_setopt(http_handle_, CURLOPT_URL, complete_url.c_str()));

  Response response;
  check_curl_correct(
      curl_easy_setopt(http_handle_, CURLOPT_WRITEDATA, &response.text_));
  check_curl_correct(
      curl_easy_setopt(http_handle_, CURLOPT_HEADERDATA, &response.headers_));

  Multi multi_handle(http_handle_);
  std::int32_t still_running = 1;
  do {
    check_curl_correct(curl_multi_perform(multi_handle.get(), &still_running));
    check_curl_correct(
        curl_multi_poll(multi_handle.get(), nullptr, 0, 1000, nullptr));
  } while (still_running);

  check_curl_correct(curl_easy_getinfo(http_handle_, CURLINFO_RESPONSE_CODE,
                                       &response.status_code_));

  return response;
}

Response Request::RequestImpl::post(
    const std::string &url,
    const std::unordered_map<std::string, std::string> &data,
    const std::unordered_map<std::string, std::string> &file,
    const std::unordered_map<std::string, std::string> &header) {
  set_cookies();
  check_curl_correct(curl_easy_setopt(http_handle_, CURLOPT_HTTPPOST, 1L));

  AddForm add_form(http_handle_, data, file);
  AddHeader add_header(http_handle_, header);
  check_curl_correct(curl_easy_setopt(http_handle_, CURLOPT_URL, url.c_str()));

  return do_post();
}

Response Request::RequestImpl::post(
    const std::string &url, const std::string &data,
    const std::unordered_map<std::string, std::string> &header) {
  set_cookies();
  check_curl_correct(curl_easy_setopt(http_handle_, CURLOPT_HTTPPOST, 1L));

  AddHeader add_header(http_handle_, header);
  check_curl_correct(curl_easy_setopt(http_handle_, CURLOPT_URL, url.c_str()));
  check_curl_correct(
      curl_easy_setopt(http_handle_, CURLOPT_POSTFIELDS, data.c_str()));

  return do_post();
}

void Request::RequestImpl::set_cookies() {
  if (std::filesystem::exists(RequestImpl::cookies_path)) {
    check_curl_correct(curl_easy_setopt(http_handle_, CURLOPT_COOKIEFILE,
                                        RequestImpl::cookies_path.data()));
  }
}

Response Request::RequestImpl::do_post() {
  Response response;

  check_curl_correct(
      curl_easy_setopt(http_handle_, CURLOPT_WRITEDATA, &response.text_));
  check_curl_correct(
      curl_easy_setopt(http_handle_, CURLOPT_HEADERDATA, &response.headers_));

  Multi multi_handle(http_handle_);
  std::int32_t still_running = 0;
  do {
    auto mc = curl_multi_perform(multi_handle.get(), &still_running);
    if (still_running) {
      mc = curl_multi_poll(multi_handle.get(), nullptr, 0, 1000, nullptr);
    }

    if (mc) {
      break;
    }
  } while (still_running);

  check_curl_correct(curl_easy_getinfo(http_handle_, CURLINFO_RESPONSE_CODE,
                                       &response.status_code_));

  return response;
}

std::size_t Request::RequestImpl::callback_func_std_string(void *contents,
                                                           std::size_t size,
                                                           std::size_t nmemb,
                                                           std::string *s) {
  s->append(static_cast<const char *>(contents), size * nmemb);
  return size * nmemb;
}

Request::Request() : impl_(std::make_unique<RequestImpl>()) {}

Request::~Request() = default;

void Request::verbose(bool flag) { impl_->verbose(flag); }

void Request::allow_redirects(bool flag) { impl_->allow_redirects(flag); }

void Request::use_http_1_1() { impl_->use_http_1_1(); }

void Request::use_http_2() { impl_->use_http_2(); }

void Request::set_proxy(const std::string &proxy) { impl_->set_proxy(proxy); }

void Request::set_no_proxy() { impl_->set_no_proxy(); }

void Request::set_user_agent(const std::string &user_agent) {
  impl_->set_user_agent(user_agent);
}

void Request::set_browser_user_agent() { impl_->set_browser_user_agent(); }

void Request::set_curl_user_agent() { impl_->set_curl_user_agent(); }

void Request::set_timeout(std::int64_t seconds) { impl_->set_timeout(seconds); }

void Request::set_connect_timeout(std::int64_t seconds) {
  impl_->set_connect_timeout(seconds);
}

Response Request::get(
    const std::string &url,
    const std::unordered_map<std::string, std::string> &params,
    const std::unordered_map<std::string, std::string> &header) {
  return impl_->get(url, params, header);
}

Response Request::post(
    const std::string &url,
    const std::unordered_map<std::string, std::string> &data,
    const std::unordered_map<std::string, std::string> &file,
    const std::unordered_map<std::string, std::string> &header) {
  return impl_->post(url, data, file, header);
}

Response Request::post(
    const std::string &url, const std::string &data,
    const std::unordered_map<std::string, std::string> &header) {
  return impl_->post(url, data, header);
}

const std::string &Headers::at(const std::string &key) const {
  auto lower_key = boost::to_lower_copy(key);
  if (!map_.contains(lower_key)) {
    throw RuntimeError("no key");
  }

  return map_.at(lower_key);
}

void Headers::add(const std::string &key, const std::string &value) {
  auto lower_key = boost::to_lower_copy(key);
  auto lower_value = boost::to_lower_copy(value);

  if (map_.contains(lower_key)) {
    auto old = map_[lower_key];
    map_[lower_key] = old + ", " + lower_value;
  } else {
    map_.emplace(lower_key, lower_value);
  }
}

std::int64_t Response::status_code() const { return status_code_; }

std::string Response::headers() const { return headers_; }

Headers Response::headers_map() const {
  auto lines = split_str(headers_, "\r\n");
  // e.g. HTTP/1.1 200 OK
  lines.erase(std::begin(lines));

  auto iter = find_last(
      std::begin(lines), std::end(lines),
      [](const std::string &line) { return line.starts_with("HTTP/"); });
  std::vector<std::string> last_headers(iter + 1, std::end(lines));

  Headers result;

  for (const auto &line : last_headers) {
    auto index = line.find(':');
    auto key = line.substr(0, index);
    auto value = line.substr(index + 2);
    result.add(key, value);
  }

  return result;
}

std::string Response::text() const { return text_; }

void Response::save_to_file(const std::string &path, bool binary_mode) const {
  write_file(path, binary_mode, text_);
}

}  // namespace klib

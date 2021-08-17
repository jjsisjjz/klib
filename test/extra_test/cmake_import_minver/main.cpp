#include <iostream>

#include <klib/http.h>
#include <klib/util.h>

int main() {
  klib::Request request;
  request.verbose(true);

#ifdef KLIB_TEST_USE_PROXY
  request.set_proxy("socks5://127.0.0.1:1080");
#endif

  auto response =
      request.get("https://github.com/fmtlib/fmt/archive/refs/tags/8.0.1.zip");
  response.save_to_file("8.0.1.zip", true);

  std::cout << klib::sha3_512_file("8.0.1.zip") << '\n';
}

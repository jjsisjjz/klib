#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

#include <catch2/catch.hpp>

#include "klib/util.h"

TEST_CASE("ChangeWorkingDir", "[util]") {
  REQUIRE_FALSE(std::filesystem::exists("work-dir"));

  {
    klib::ChangeWorkingDir dir("work-dir");
    REQUIRE(std::filesystem::exists("../work-dir"));
  }

  REQUIRE(std::filesystem::remove("work-dir"));
}

TEST_CASE("find_last", "[util]") {
  std::vector<std::int32_t> v{1, 2, 2, 3, 4, 5, 6, 2, 5, 4};
  auto iter = klib::find_last(std::begin(v), std::end(v), 2);
  REQUIRE(iter - std::begin(v) == 7);

  v = {2, 1, 1, 1, 1, 1};
  iter = klib::find_last(std::begin(v), std::end(v), 2);
  REQUIRE(iter - std::begin(v) == 0);
}

TEST_CASE("split_str", "[util]") {
  const std::vector<std::string> std_vec = {"123", "123", "123"};

  auto result = klib::split_str("123abc123abc123abc", "abc");
  REQUIRE(result == std_vec);

  result = klib::split_str("|||123?123|123!", "|?!");
  REQUIRE(result == std_vec);
}

TEST_CASE("read_file & write_file", "[util]") {
  REQUIRE(std::filesystem::exists("zlib-v1.2.11.tar.gz"));

  auto data = klib::read_file("zlib-v1.2.11.tar.gz", true);
  REQUIRE(std::size(data) == 644596);

  REQUIRE_NOTHROW(klib::write_file("write-file.zip", true, data));

  REQUIRE(std::filesystem::exists("write-file.zip"));
  REQUIRE(std::filesystem::file_size("write-file.zip") == 644596);

  REQUIRE(std::filesystem::remove("write-file.zip"));
}

TEST_CASE("read_file_line", "[util]") {
  std::string_view content = R"(aaa
bbb

ccc

 dd)";
  REQUIRE_NOTHROW(klib::write_file("write-file.txt", true, content));
  REQUIRE(klib::read_file_line("write-file.txt") ==
          std::vector<std::string>{"aaa", "bbb", "ccc", "dd"});

  REQUIRE(std::filesystem::remove("write-file.txt"));
}

TEST_CASE("utf8_to_utf16", "[util]") {
  auto utf16 = klib::utf8_to_utf16("zß水🍌");

  REQUIRE(std::size(utf16) == 5);
  REQUIRE(utf16[0] == 0x007A);
  REQUIRE(utf16[1] == 0x00DF);
  REQUIRE(utf16[2] == 0x6C34);
  REQUIRE(utf16[3] == 0xD83C);
  REQUIRE(utf16[4] == 0xDF4C);
}

TEST_CASE("utf8_to_utf32", "[util]") {
  auto utf32 = klib::utf8_to_utf32("zß水🍌");

  REQUIRE(std::size(utf32) == 4);
  REQUIRE(utf32[0] == 0x0000007A);
  REQUIRE(utf32[1] == 0x000000DF);
  REQUIRE(utf32[2] == 0x00006C34);
  REQUIRE(utf32[3] == 0x0001F34C);
}

TEST_CASE("utf32_to_utf8", "[util]") {
  auto utf32 = klib::utf8_to_utf32("书客");
  auto utf8 = klib::utf32_to_utf8(utf32);

  REQUIRE(std::size(utf8) == 6);
  REQUIRE(static_cast<std::uint8_t>(utf8[0]) == 0xE4);
  REQUIRE(static_cast<std::uint8_t>(utf8[1]) == 0xB9);
  REQUIRE(static_cast<std::uint8_t>(utf8[2]) == 0xA6);
  REQUIRE(static_cast<std::uint8_t>(utf8[3]) == 0xE5);
  REQUIRE(static_cast<std::uint8_t>(utf8[4]) == 0xAE);
  REQUIRE(static_cast<std::uint8_t>(utf8[5]) == 0xA2);
}

TEST_CASE("is_ascii", "[util]") {
  REQUIRE(klib::is_ascii('A'));
  REQUIRE_FALSE(klib::is_ascii(static_cast<char>(190)));

  REQUIRE(klib::is_ascii("AAA"));
  REQUIRE_FALSE(klib::is_ascii("你"));

  REQUIRE(klib::is_ascii(klib::utf8_to_utf32("AAA")));
  REQUIRE_FALSE(klib::is_ascii(klib::utf8_to_utf32("你")));
}

TEST_CASE("is_chinese", "[util]") {
  REQUIRE(klib::is_chinese("你"));
  REQUIRE_FALSE(klib::is_chinese("a"));
  REQUIRE_FALSE(klib::is_chinese("🍌"));
}

TEST_CASE("base64_encode", "[util]") {
  CHECK(klib::base64_encode("hello") == "aGVsbG8=");
  CHECK(klib::base64_encode("Online Tools") == "T25saW5lIFRvb2xz");
  CHECK(klib::base64_encode(
            "How to resolve the \"EVP_DecryptFInal_ex: bad decrypt\"") ==
        "SG93IHRvIHJlc29sdmUgdGhlICJFVlBfRGVjcnlwdEZJbmFsX2V4OiBiYWQgZGVjcnlwdC"
        "I=");
}

TEST_CASE("base64_decode", "[util]") {
  CHECK(klib::base64_decode("aGVsbG8=") == "hello");
  CHECK(klib::base64_decode("T25saW5lIFRvb2xz") == "Online Tools");
  CHECK(klib::base64_decode("SG93IHRvIHJlc29sdmUgdGhlICJFVlBfRGVjcnlwdEZJbmFsX2"
                            "V4OiBiYWQgZGVjcnlwdCI=") ==
        "How to resolve the \"EVP_DecryptFInal_ex: bad decrypt\"");
}

TEST_CASE("md5", "[util]") {
  REQUIRE(klib::md5("MD5 online hash function") ==
          "71f6cb39c6d09c6fae36b69ee0b2b9cd");
  REQUIRE(klib::md5("SG93IHRvIHJlc29sdmUgdGhlICJFVlBfRGVjcnlwdEZJbmFsX2") ==
          "ee60cfe37f9a60b9ceba008be6f1c034");
}

TEST_CASE("sha_256_file", "[util]") {
  REQUIRE(std::filesystem::exists("zlib-v1.2.11.tar.gz"));

  REQUIRE(klib::sha_256_file("zlib-v1.2.11.tar.gz") ==
          "143df9ab483578ce7a1019b96aaa10f6e1ebc64b1a3d97fa14f4b4e4e7ec95e7");
}

TEST_CASE("sha3_512_file", "[util]") {
  REQUIRE(std::filesystem::exists("zlib-v1.2.11.tar.gz"));

  REQUIRE(klib::sha3_512_file("zlib-v1.2.11.tar.gz") ==
          "38af19362e48ec80f6565cf18245f520c8ee5348374cb0c11286f3b23cc93fd05a6a"
          "2a2b8784f20bb2307211a2a776241797857b133056f4b33de1d363db7bb2");
}

TEST_CASE("aes_256_cbc_encrypt", "[util]") {
  std::vector<std::uint8_t> iv;
  iv.resize(16, 0);

  REQUIRE(
      klib::base64_encode(klib::aes_256_cbc_encrypt(
          "{\"code\":\"100000\",\"data\":{\"login_token\":"
          "\"06d3b540ecde7843d79fa0c790b4c968\",\"user_code\":"
          "\"9827638bc3c6ae0a43174f2a2d25d35b\",\"reader_info\":{\"reader_id\":"
          "\"9986391\",\"account\":\"\\u4e66\\u5ba287999639162\",\"is_bind\":"
          "\"1\","
          "\"is_bind_qq\":\"0\",\"is_bind_weixin\":\"0\",\"is_bind_huawei\":"
          "\"0\","
          "\"is_bind_apple\":\"0\",\"phone_num\":\"15041557811\",\"phone_"
          "crypto\":"
          "\"HOcCgi\\/"
          "crmKmnAKvlSoZbQ==\",\"mobileVal\":\"1\",\"email\":\"\",\"license\":"
          "\"\","
          "\"reader_name\":\"\\u4e66\\u5ba287999639162\",\"avatar_url\":\"\","
          "\"avatar_thumb_url\":\"\",\"base_status\":\"1\",\"exp_lv\":\"4\","
          "\"exp_"
          "value\":\"697\",\"gender\":\"1\",\"vip_lv\":\"0\",\"vip_value\":"
          "\"0\","
          "\"is_author\":\"0\",\"is_uploader\":\"0\",\"book_age\":\"1\","
          "\"category_"
          "prefer\":[],\"used_decoration\":[{\"decoration_type\":\"1\","
          "\"decoration_url\":\"https:\\/\\/app.hbooker.com\\/resources\\/"
          "image\\/"
          "decoration\\/"
          "pendant_82.png\",\"decoration_id\":\"82\",\"reader_decoration_id\":"
          "\"2631577\"}],\"rank\":\"0\",\"ctime\":\"2021-06-04 "
          "16:43:55\"},\"prop_info\":{\"rest_gift_hlb\":\"10\",\"rest_hlb\":"
          "\"10\","
          "\"rest_yp\":\"0\",\"rest_recommend\":\"2\",\"rest_total_blade\":"
          "\"0\","
          "\"rest_month_blade\":\"0\",\"rest_total_100\":\"0\",\"rest_total_"
          "588\":"
          "\"0\",\"rest_total_1688\":\"0\",\"rest_total_5000\":\"0\",\"rest_"
          "total_"
          "10000\":\"0\",\"rest_total_100000\":\"0\",\"rest_total_50000\":"
          "\"0\","
          "\"rest_total_160000\":\"0\"},\"is_set_young\":\"0\"}}",
          klib::sha_256_raw("zG2nSeEfSHfvTCHy5LCcqtBbQehKNLXn"), iv)) ==
      "IT+LcNazRBcK54/"
      "p1lMtcyRwpZ01VQ4tFr6GBslpnwMezmEBbIYc3GokHiTGB6XV/"
      "I3oWkrqLXB3DTQJUIlvLRRHe2GmNqGS8xHXeyq8BgBLCBxLcIFRtK+V6/"
      "Y1ovce7ie8h4t59PU0nHYbr8Lh7gq0yZ3DRd5oJ8go4QPgtTTPdfXCWIGMIbW"
      "ot0HwcYIGiCxE7RcvSEBdti5MBTmVHeF81cQYmB9SOPnxt4KOjH557P2Y6Pya"
      "nzcVO2BKpGvCgcVSHmkwA4xX1JjH8zjZ7miuFmnNKDrDacmx3AcxxwbtJpog+"
      "loA/b4hnDny3oCKkriy0eHkc5atGAwg/"
      "HFkArM2wUXZtUHuRWekImGlapISnp1fVpgvf2WKf5ENt8/"
      "LJUY8kX56wE8tea6feekOO1U1fiQyGHf4yceoeZTXfKHs/NXb9/"
      "YbStpsSaMvLujmbR0j2sBpMLyAqUAZF/1tEMlo7od/"
      "SS+AXj+T7R7daLFAKHLfb+gTqB/"
      "WffPV0SeG7dcDLKL7dvno0MEBvDP1RKBG9ACxWHAGq94GlBQcimH9xNAmAGeQ"
      "uyh9R7TmWVtAgSfW1q1tIpU22yytgXW3grdl0e1kqFr61PxXPiKEwFBlSUA/"
      "OJgFswAOayx6aXVdazd05w7QcXyexzka6DPKjMUi/"
      "Cw88uk6ovHmSffnx7hzoFz7qcGWO5rHuILxIin1HJjbNOqb/"
      "7IEnsjIUJxTp4V3ula9DcR3VUbSDFFo0oV98SNTSbSe8MBpTX5AREIN4Yr5SS"
      "vUji7Mm0EjlwOgIgzgcXZgCEGQty3aI1mj6luaCxWAe4YoJhJrr2ZHZxWPgjC"
      "we7dOSGWQHGAEEF9RiqX81yAqay13uYThwgMwIAxzA4TrevdoTgDuo/"
      "L2dvoVQ3UItVrAGFBZhFL6Xhi0izYjpXbpVWg0lo9M/"
      "rgBdysSYGrDTYXq0wVmEhJaPhCw9bwKqCK/"
      "uf1Ad+bqktuwCouikVmIfTJxV8qlBeoklFtrUuS93Zmyzq5Cj2CDnEH6+"
      "8j0ypE/"
      "wVnq1UuIPEtn9xDfVXSoxm8QcSAJ728jrRLn7g7QWCrB+"
      "eGvCmVqzo31GaF6I642OhFTyuLUmsLuR8aONaMOueuiFQL/"
      "dxdeP+KILutwIH0Mtg8jS0bWUlN14+"
      "LJLzuqlE3j7Cz6HV29CI9ri35WTDvn1TKqF0rama5bRt5Jp+"
      "WRrXXEELbTIwjkaNfY9bBMM75B8ewIaOpOWXPaAI1vLejz6r5aSAsz26bGMWZ"
      "J3A8s+OtcaqeIcTTtjB9N/+qVGyViyOJb/"
      "YHS4cYH+"
      "pCj2wZR4s1m3TrdLNSw5VGBBPzLUQtBMSungUNS7knFBHbyQMwtoD8tRsHdGC"
      "DCoiAbWOCwUQFeld9MpcuDGRE93gUo8Wz4GHjsm8V+"
      "WrsD7sW5bRxngbdrUSiGJhbkHh+I+rMqNDkEGGZ5eGM/"
      "s3ZbTuChHMCAh75IY1an6E2QJddEOBRfszEtWcGcy+u9ACV/"
      "hXhhg2mw2KUT2QnyJ9en9jPkjEOd04V8ja3Aoqk3chTTQJksU8D/"
      "XLOyDgEkWNaX5I4WgTtkOeLyEQVg2yzrAwjgayCXWj71JDe4");
}

TEST_CASE("aes_256_cbc_decrypt", "[util]") {
  std::vector<std::uint8_t> iv;
  iv.resize(16, 0);

  REQUIRE(
      klib::aes_256_cbc_decrypt(
          klib::base64_decode(
              "IT+LcNazRBcK54/"
              "p1lMtcyRwpZ01VQ4tFr6GBslpnwMezmEBbIYc3GokHiTGB6XV/"
              "I3oWkrqLXB3DTQJUIlvLRRHe2GmNqGS8xHXeyq8BgBLCBxLcIFRtK+V6/"
              "Y1ovce7ie8h4t59PU0nHYbr8Lh7gq0yZ3DRd5oJ8go4QPgtTTPdfXCWIGMIbW"
              "ot0HwcYIGiCxE7RcvSEBdti5MBTmVHeF81cQYmB9SOPnxt4KOjH557P2Y6Pya"
              "nzcVO2BKpGvCgcVSHmkwA4xX1JjH8zjZ7miuFmnNKDrDacmx3AcxxwbtJpog+"
              "loA/b4hnDny3oCKkriy0eHkc5atGAwg/"
              "HFkArM2wUXZtUHuRWekImGlapISnp1fVpgvf2WKf5ENt8/"
              "LJUY8kX56wE8tea6feekOO1U1fiQyGHf4yceoeZTXfKHs/NXb9/"
              "YbStpsSaMvLujmbR0j2sBpMLyAqUAZF/1tEMlo7od/"
              "SS+AXj+T7R7daLFAKHLfb+gTqB/"
              "WffPV0SeG7dcDLKL7dvno0MEBvDP1RKBG9ACxWHAGq94GlBQcimH9xNAmAGeQ"
              "uyh9R7TmWVtAgSfW1q1tIpU22yytgXW3grdl0e1kqFr61PxXPiKEwFBlSUA/"
              "OJgFswAOayx6aXVdazd05w7QcXyexzka6DPKjMUi/"
              "Cw88uk6ovHmSffnx7hzoFz7qcGWO5rHuILxIin1HJjbNOqb/"
              "7IEnsjIUJxTp4V3ula9DcR3VUbSDFFo0oV98SNTSbSe8MBpTX5AREIN4Yr5SS"
              "vUji7Mm0EjlwOgIgzgcXZgCEGQty3aI1mj6luaCxWAe4YoJhJrr2ZHZxWPgjC"
              "we7dOSGWQHGAEEF9RiqX81yAqay13uYThwgMwIAxzA4TrevdoTgDuo/"
              "L2dvoVQ3UItVrAGFBZhFL6Xhi0izYjpXbpVWg0lo9M/"
              "rgBdysSYGrDTYXq0wVmEhJaPhCw9bwKqCK/"
              "uf1Ad+bqktuwCouikVmIfTJxV8qlBeoklFtrUuS93Zmyzq5Cj2CDnEH6+"
              "8j0ypE/"
              "wVnq1UuIPEtn9xDfVXSoxm8QcSAJ728jrRLn7g7QWCrB+"
              "eGvCmVqzo31GaF6I642OhFTyuLUmsLuR8aONaMOueuiFQL/"
              "dxdeP+KILutwIH0Mtg8jS0bWUlN14+"
              "LJLzuqlE3j7Cz6HV29CI9ri35WTDvn1TKqF0rama5bRt5Jp+"
              "WRrXXEELbTIwjkaNfY9bBMM75B8ewIaOpOWXPaAI1vLejz6r5aSAsz26bGMWZ"
              "J3A8s+OtcaqeIcTTtjB9N/+qVGyViyOJb/"
              "YHS4cYH+"
              "pCj2wZR4s1m3TrdLNSw5VGBBPzLUQtBMSungUNS7knFBHbyQMwtoD8tRsHdGC"
              "DCoiAbWOCwUQFeld9MpcuDGRE93gUo8Wz4GHjsm8V+"
              "WrsD7sW5bRxngbdrUSiGJhbkHh+I+rMqNDkEGGZ5eGM/"
              "s3ZbTuChHMCAh75IY1an6E2QJddEOBRfszEtWcGcy+u9ACV/"
              "hXhhg2mw2KUT2QnyJ9en9jPkjEOd04V8ja3Aoqk3chTTQJksU8D/"
              "XLOyDgEkWNaX5I4WgTtkOeLyEQVg2yzrAwjgayCXWj71JDe4"),
          klib::sha_256_raw("zG2nSeEfSHfvTCHy5LCcqtBbQehKNLXn"), iv) ==
      "{\"code\":\"100000\",\"data\":{\"login_token\":"
      "\"06d3b540ecde7843d79fa0c790b4c968\",\"user_code\":"
      "\"9827638bc3c6ae0a43174f2a2d25d35b\",\"reader_info\":{\"reader_id\":"
      "\"9986391\",\"account\":\"\\u4e66\\u5ba287999639162\",\"is_bind\":\"1\","
      "\"is_bind_qq\":\"0\",\"is_bind_weixin\":\"0\",\"is_bind_huawei\":\"0\","
      "\"is_bind_apple\":\"0\",\"phone_num\":\"15041557811\",\"phone_crypto\":"
      "\"HOcCgi\\/"
      "crmKmnAKvlSoZbQ==\",\"mobileVal\":\"1\",\"email\":\"\",\"license\":\"\","
      "\"reader_name\":\"\\u4e66\\u5ba287999639162\",\"avatar_url\":\"\","
      "\"avatar_thumb_url\":\"\",\"base_status\":\"1\",\"exp_lv\":\"4\",\"exp_"
      "value\":\"697\",\"gender\":\"1\",\"vip_lv\":\"0\",\"vip_value\":\"0\","
      "\"is_author\":\"0\",\"is_uploader\":\"0\",\"book_age\":\"1\",\"category_"
      "prefer\":[],\"used_decoration\":[{\"decoration_type\":\"1\","
      "\"decoration_url\":\"https:\\/\\/app.hbooker.com\\/resources\\/image\\/"
      "decoration\\/"
      "pendant_82.png\",\"decoration_id\":\"82\",\"reader_decoration_id\":"
      "\"2631577\"}],\"rank\":\"0\",\"ctime\":\"2021-06-04 "
      "16:43:55\"},\"prop_info\":{\"rest_gift_hlb\":\"10\",\"rest_hlb\":\"10\","
      "\"rest_yp\":\"0\",\"rest_recommend\":\"2\",\"rest_total_blade\":\"0\","
      "\"rest_month_blade\":\"0\",\"rest_total_100\":\"0\",\"rest_total_588\":"
      "\"0\",\"rest_total_1688\":\"0\",\"rest_total_5000\":\"0\",\"rest_total_"
      "10000\":\"0\",\"rest_total_100000\":\"0\",\"rest_total_50000\":\"0\","
      "\"rest_total_160000\":\"0\"},\"is_set_young\":\"0\"}}");

  auto base64 = klib::base64_decode(
      "TC64Ykj+"
      "HY4FfD2Ddh8gBHEcR4IdUjLbfrb44QJPeC43dvPghqXE6gDdJSip1pzHxtfC5cBkR1lu"
      "JvxYnfX8QY4n+Pn9ATexdSyPtJhq90OTs9vYNcsnFIfj4VPfHa0/"
      "4KafU7bMwk6HRlg+d6dgDSCcJaDJVuubJzOocDic0hQN4u897kDKv6UCZS+"
      "HnEit8rKtfyXyhmjeiTblkPmOe7C48YF8k+"
      "AevwwSCXVorn6dAqyZ18lIuSZazp94Rvsto9X11tdGPPUlxAALHDi2Yf8YBovnkJLC0P"
      "PQqv72+yKRKAS7slpWWJiuAGsTUMyglbpc70/0cV2d4Bd0A7y5ELh/yZCWj/"
      "7JgrPQHQj4Si24XX0U6qh3oGoRMDOIiOEqtY0RXT6T93G5wYMZX97sZVroh232NEzmwQ"
      "Imf1ow0LbF02YEo1+4LY90m0bR9ZVITsHXK+3y6qsX9IgYrPUbhUjl10g5GDj/"
      "DKtVRb6kvubWE9tpVSWV0RaLglNUKBWX5fwGj1JsLSbCbnanZPSW+"
      "vpuh1RqquYOX2otth2A2XWLyIupbxkn1sBLHlRdPv5lSVzPnognCTK8mqBnUZkkhv3LG"
      "uiu36cDQy8jbqxiO16HqKkpGOxdCbjVY0l3SYtyWu6pZgmJCn6SxctBrhtmUnKuvyhsg"
      "i0wcFjIqfIU3i2QMuAwWICpA2E/"
      "fwqy0ebsyBrVeU+TedpsG0dLzzTF+rfCmKM0FhokFqrPjp9R+825YiZSQc7meOB+"
      "cQbq2rIDM2NtbEXdAIc4qVIBsarvk2BDetdh/"
      "UHbAENbHxp2zulrga6rPEv1Vs2TTpREqHfTuuYwB4hE09G3OPwveVaSxE6LaazJK/"
      "fJrud4+yAgH5C4vCMhaPLjYQjvXMBTdjuHIa6AQrsZNteLFGzRe/"
      "cFryJ4A0ni0d2J88TPH2ZPy1zMQ5cgbyQzMvoQ+OzIeqifkHwz90/"
      "Uh7mS3PF3584k7DNb/ozhuwyw8B/vkyvAT/"
      "cWJrUdHQjN20x05RrXZUuEqmbXMnUy9khxsy7CqMoGeTmNNwE22+HYz16KDzST5/"
      "W925cEJXJYD8Crg2ZDCQwEsYDnrg0bfH2ss6g7iGKzUXXWdD6U1FS0D5FuVLddPU2OVY"
      "qrTMVTepL4a4p2eo0M6tdtv8udcT8BMCZbuiOt/qG20jVN/"
      "Tx8gWwDtnSTB6dLoOSb1pL/k//PS70tfYAcr/xPXgh7lh9TBXGkJfhgi/"
      "G7ZLbyNKvGEbrQxewe6Z6a1Dy0dMIrTUOMdplc2IWKNOsvQBRLBKZr02z/"
      "+XL+Q5Bg+Zhp38yF7hhT5cgkIhnunH6IItPb+edtGZTYtQ3HT088MBr/"
      "FlSIXdeWFvDJzcHA/"
      "g+"
      "NFluBZ3C0e7XfO7PMPkqJKbkT89wtl1JBUX0meZ6MjnK7ya1wQkNy24CC9vaOlVS69To"
      "t4K2Ip7LeEZldfQ53EAJ4T8lAdnIaoz0vh8XZM2zxD3pJHQ2a3p1xM9dQG3c2qJEMA/"
      "qaOb2wdiYmqqe3+QBAnUJ42CvFXPL24dAuKU03aTQ3b7shLRpDR9MKLy3DrxL9Wl+"
      "GNO6W45bR8kohSO35y/nLsdvUT16zThlYpyTdoqp2dKI/"
      "oNgtMSAy9fOn1lTooeL0dex56uVLNiGC9Ekeyw7c5Zo6YDSR+"
      "w49uMJdCaEMq9ofU2Dg/xFQvKpY5wteRibITFzC1dSx4arPRoPFxar/"
      "fK9X8177OK9o7mkUU/"
      "LvQ+Kgr6dR7xu2UhWqBUld7l6HRjSbhUvbmIH6zFD5SNJzqZm9z8zpXaRl63Ekllz+"
      "zU28XQcsXKTMFAXjqmoTTAm+UygApPPyYlkCm63+"
      "KlGHGAGDpuJCajVggqDYmKN2TUu5jlTZwKKcS8+trdk0jfi5uLMgfUelkFLZQRbdCZU/"
      "EsbLJVoOSiBxEGJXqRyLtm11V4hxWQBfmdq9T/"
      "N397IjnXHL24P0dUB5jHUCjvACQL75eoOFT3viSPOTIfDY+"
      "xNFcU9ozgoA7tzTW8S0K8uEmuCJ+3XORJbrr8g6qH3Zj0fbWEWovYr+HuadiZ/"
      "F4afRqFu/f5FCZ0o2mAud9XHnOYwrrDRbjuMXkzSZR54q4sU3/"
      "z6nOvr8X+2s1nTNY5jK8jIVLo6jG1GFEPdpz2nt3+"
      "L9PmjSisoX4Mjue9W5AKFoG9MjoWMiHgtD8Jr0kgWp5mjqcnKimEniEeOqS56rkjsHry"
      "zCsfyoDKfuvM0+"
      "o6zL7nLZDNuof8NeseAoTBIwNXGrfWUuRUliSNDTj8vX3mMrGRk0RUZsgxW59ZQpCtyX"
      "ThUq485V6z5ln+J+"
      "gHyI3TBgnPn7bkCVgkLr5Sm0nA0J2Mflavo2xsKQw84QCnWOKwhKDYMKEaPRp3N2bs3B"
      "8CUZCMxQla0qjUYVwv/cnUZ9GAPyzHWW63H08+cl03ZfnIy7dUdfN654gjquRWGd/"
      "FHfxFSheygNAMsTLvMs74egpXnY4pBPTVIuhyFycpVMdIPwvEd9qTTbIbLbJXHlEdeXY"
      "mEOO/Ib5lJDS36a+EOtqyHGf6ONk2Tx6mQQNigUXA+ILABnoJchKVZup5E9pSVaL/"
      "yEVLvaqEuOsTRAkC7la9Q2rOrhtNjZMnJMbCHdjYw3gj3dvHgC9VqAYFkQAGu+"
      "QpDFG3+DzfgkDStCH5NLdLe/"
      "vvv6xhGcm6vHUIMKQ8y8fgrvMMTpf8aZk4aTV+"
      "eVY5qj5Z1FPOYXlXfHHAD1PxZH6tPUxJz9tN+DZ/"
      "rareTMLQH4W3JqsOTjg9xnDZJqeNAczJim4BuZHr+3OkMQ+jvTLJzalk+"
      "au4T6WRZ8bUUoQsVtnO4QDScvDxEoqTbv4QFQmUF1zi/"
      "cxXip49taBxIX5SgcUZjK0wb515fuRJGdo+"
      "hDBWji96IfkdYVT49qIvRvcm4bXuImwoxDTdVafxxEsZzBb7zJA0LzFx1eqpiNbO8lRx"
      "YHH5y3CRz0Uo9eL7MnO0P9yWpjli7cOgRPULmyhpV6oAsbCinIbBaFZhGgSAz5hGqhiF"
      "o9N5FyozYJbO6DJvfo+me7B538z6b8jYUQ8Pn+"
      "goDhSUN1YvuFccRIQQvE7PcbES2ykCvn7SqndcWJ9TDBm85FhY/"
      "YuDmu1XtacPru17YirlPIEPPqldft7oTFl4pl/"
      "+socIGDVizKLXuylJLPtGbcpjO65qt18txspiCeutNaB1utG5haJzF+"
      "avC72WN7T35CwvkrvY61VNLQTnOQ8r+"
      "Gt7dJEcFW0COEWV3U0FlqWTeyREt5PlDYYmUqerVzJ66feW9OO66HODwqPvuE01jJ13v"
      "J8avrn9miz0lLCU7k+"
      "m2rbrAm1CaYMdOycMQCi4RQn1mdugpzWQx1iGirzz3C6IavtvstRUrWpQ/"
      "lsibcYvWLllCXdkf2ehSQP0nNC6rlAdpxgF76PHoN9rDqjtbRApMu9XjWfv0aI/"
      "UM7Gzh2aDo+"
      "d99QlMCJY7aIk75eeqTfSSP9mp8qR9G7zUusMTjp8mwVHPyAxypLVuMT0QnMehQvxR/"
      "1YYzY08gmxahlQV+"
      "BIXGT7pH7qtpf8btwSzMQ0z1ZGsK5xe8LKNpQK9tDIRMTSlqyjmimFS3LwsbF0J5BJG6"
      "b19VpjHYKNBvEAtRqlTZX8QSktrp4liQxCfQBu6PeiCUAlS3HxVfv3nVCryZAtW7w0h4"
      "KG6nu8UdiE2qbWBaY1ifZAtcIpLhmo0iGNPTZg5+"
      "r2j5kRnNEjOBWDyzASAVHV0OW4d5bez7gGje2X8SftRgkk8hSMD1fqH+"
      "UWLtEC45MUdZmMBDS9wyCmH7VlwjpqZzJOSSz3sggHJ8K1PhoNZ6jh28yz8u6I71Va+"
      "KWbDbmU5LlxxLHhq/ohMZrx65AYo9HxdNgQii7nYqamMHJFDgajCqsKtPhk1o8dOXG/"
      "zNoKlM3uN/MFMKLAzXqPTt/V/"
      "4nJjplL8j1sHUnV3nPxtDGg1oRDHO99tKWsCyGKbY+ha9k/"
      "NsNhZkVmyfHV2dgsXdZeT+"
      "3STnyqxYj9PIbcnueQxpcGRbYUP4Do4GcGz3TIt6LpoPwQbngpVm7Qag2ExuozWG1Zte"
      "579Xjtsc9cI8WZb/7GjrResJ5yr5F+WZ3T5ZTALdvL5/"
      "Ki8QgnXTOGeXJtobQzdY3g02xpR1sX8+65cJqTDYNdfnp5Q0+"
      "5S55746fejoXrYEHwxuUVkQtwHMJ4Y3oUMdbN1nAAVx8HVFMKsR4hYXlE3VOFCQBJEpL"
      "L+"
      "aDF5aeViaDejt8uRg2d1j7iIik0KE5ekGsk3BLpiWzTSRIQWnaTN7JwIaWlTadgRFL7q"
      "idVE+LyGtEBZpxIOwPMjKexRwI0Amz8McnnrJzT56GhvzYKkKjrdl9+eo/"
      "J3gKYTFLUIJyu488ODFxiNufMGy1dzxC39hlROJLTzzXTYd03CxnoHsfJqiaAJUcLbjX"
      "/xmqC3Hq5Yb+rgtiIY12/0/"
      "8CbU1YMgYHvsNbEb9qSSRdzqaGMrwWmaxkwh8jw5uzSeucvUeArXLlAiSfispekj54vy"
      "Gs8RQaGog6YQyyeUcLXVxDcKND1tJ4j6YySN9hFrGhITKqBYqcHJIBihUdXmD1cd1Gw7"
      "UCW0meZbk7OcUT6vGFwE038Wf0fP3aM8eotma57P1VGxfoxvM4Gbedp+"
      "QEMQBFBVJ8Mhk/sZVJB5jv/nBDTY0AFbiHqh6dCNqQhy0NX/"
      "kHD6OhUM0VsM+PsMe1w4JZVps/"
      "Ln0eTwLWdTzi3MkB5JrVTQCG06Woo77ojJQr6bEpXJRMDBYXrND7gStFTphQUZ+"
      "zWKpW+JHscwk48v3h/"
      "thWxLHq+WhEen0tOEPRM2O96jPt6k6Yked73BfhWFo0jfThYI+JNuRy0JtjJr4JKk+"
      "jN05RdRcJXYBksV8oeuX3q4WPblFGtkcsAkYbASiwoKUT0FuyLHW94nvosjKo1I3IM53"
      "2dj2knF/mXA4JEipFmy+/ZNUHEvIUZkB8Ow3cA3S7qDicmUmTV6Qgf+E5uxizmcNy5/"
      "KLeiK367zf5miYptTbcc4u9APyzuOVLRxzUKPMeaWmo95NNUDQrpksWm87vXcMtVMSJU"
      "52/sAg5E58U2AvGzVx1sr4OAMd8fHerHAi+5LsBgFwIGKsNdQcJnl3YyUu2/"
      "76CUiudNJi49CXTT+"
      "sVRsPtWlTFxpa2WOSwMkn3Y4tjCRGT8ydWdqkLx0gtBdnu8eRuSVTU+"
      "baw1R3v9HCFUts/SI3hBcDv5ytJ9KBLSqDd1pO/QHtpsHzAi/"
      "KwJSne6QrtXUlgu10iAUCsVRg8/Fqe/"
      "Q3YGEiBc2GVaKaDCjGrFtFnk7Cc36kNWVJlIJtt09cVvrfl4fONtO3K8+"
      "HZYcOPfeDAhRCrvDS8lQNhQUxO8LT84sWR65+/"
      "GEOEfqWeVosovkMLo7Op+HmIEH5RimMgYowqRxSb+i3R0YWnW8a8Q8I/"
      "KRNnXk1n2IWp3eI4JMQBsG+"
      "U47fxV3K4GPez7tcnquCwYp1qDeDBgUhPG0v0A9VAmluzBaslQ176Q+EVP93Y/"
      "HmMnC1kt3GKjb1C5Hv/pwRa/"
      "CAu+"
      "6gO1mtx4Kuc8DUFHkRqrqSkt93bvVvAy08YTqXISOU470J5YAE569tlylF0XX2GRrqzb"
      "l2nqhXtpEeAoWL56rJSnx29MigxtLTlxdxIt14F/"
      "Ad81C5lRNckeOxCHe4IU1cqIrHVlUeJxIg1DSLVbTUqLr3KAfxcUHKRAcXbCRsO/"
      "zyPHtqLuVBNjDbdQlgLTNNUzZwLknw5Gy10tj7rwFbnpGP4HFzyvdzCbtNhHRmsduHe6"
      "bdo+c4FShy5un2DF7YtkfDSdUbuaWqjeYUA6p4a1X1hf7teoPkIkzvzskoAuvlWO/"
      "9TKRt47XIvsnkkC9/DH7YqOETeQ/"
      "THt37uiUcwyaFGyAyZC0rPRDv22U5RtOkOWI9PKlI1CqRG4a1bKWvIPBBn4biNd0qaSd"
      "fSL3CWGiQ3qqFrrS3CMTQciLkKlwFRIfyf/IAUfEVp9bZjv4/"
      "uOVCitsUSHUcIQkXVb8oErHXJLF7VVfL+IWDPRV3y+vakQEib3XXU/"
      "nmeAfeOOj4ChWgcvWf9fds/Ni1nh5M3Km5jV8TuDHEcc0hKek6/"
      "qdViMRb2niw6+"
      "V1XDyQZrzmHyzBtJJVnoDPl7inqKDbSCtBYEJEc8NQCIoOduF2u7Ha1fz8rwKi3gkPkS"
      "+ZKIzGBHOo3y2J0ATtmZUeAWG8NF/"
      "qbkZzbv9EfXIk9Tfd12TccsUoLt8CDbb5ckaG6kEWA5Vo6Z5fA1b17e05kGIjEj/"
      "rRwSGg5eDjKNR42yF4r2GDersp99xvdKvZMc3e6O82xGsA8k0tg3dynAXGu9h0bs3AkO"
      "QVO1/yMnqDFFurxVDZDzPRd1jddnF4Tie52QrQsqj/AD1FmKe0wRVeyW+JfgAbb7S2/"
      "zr0aujm+OPjV+9StFqMjur7xgMNwe4VlyXTuAT0FYSwBeX1JTHZll6LYI8JT6V+"
      "GE4QjaSh0bGKLAUejjSR+51EJRjknMMPh4O4wx+"
      "VwN5Xp7KLmUaiZ1BPhtjUDf2UgQZwwT10urMQyouCfQ3B3hbFvrED7/"
      "MomGWoUXL98rfRXQ1wa5ZoTLK63ITKAq3JWEoK72A+RdWCASY9lUO2+pY+t+"
      "p1GxYFR7PIcbt89h8HEWlBFIN0GYqpIDbe748uEsCponILEDtCodgH3krHu5e5pHlPTo"
      "LkTF1Pl3GwVr96eH/"
      "w0sMz8+Q2YBN+"
      "Nj5gJ7cKFK48drQUyDCAFRs1YBD6qSo1pg3DZIiwuw4lxOdbrRBbjXfeBQyizPw/"
      "BNiRRuS0OtBB1A2J8/"
      "UvFKqsyiJkVIEajRtGWmlgcv4wf1vDcqPv6cI5Gl++RxRrFyqd2am8/"
      "Mb8ZATKoO5BXy7TntGOKjoDxUaztd6eXZvI6TPPXZsfL986V3vDFavM/"
      "83kQIbM1Qe99VyvM/2/6hEYeGItg7/"
      "1+4mQJE0iGpgKnXT0DZTKJE5I00UR68nfWXkTH/"
      "xf8bzud+wci8Ysl569QcERtJZwxjF05WtaQ8aIrOjwJz1RCZkzcp+"
      "elaa6pjy0XtWwsafLDE1LzcKecl5Od/"
      "7vuxMm0jiJ3WraxVU7ofguF13AQkNrxhSpwciCzvirS21L2IHMqW/"
      "NrNC9PA3fDgtVKVjM4lmdbuBeWi0Bx64fwMskxQJ+"
      "Sj5VrO6TUIjaU6bH22DtzDJUyKlP908vmCtLzdqi/"
      "PNTg9sgX+xdmpAWeiOvCGbhyeWLVXvhOKaWSWrsHgkl/"
      "MuJVD5Wezs2bfeoI7KUQ8Z1LCzEHiBRe/"
      "P7+HoSU+O4vtBo6Q+"
      "9H8bHIyLHz9ewVnQ1UZIsxvXj94oYkbghw01Qa2nDO3c0Z7U2QVfQwgNgyFG40cqd4C+"
      "m3nALlxI5NC/+YOQ832O+YKHpsuLX0AgBzyZuFDy/Ob/pe7XYeTPiBZwOdD/"
      "JLVjeaLb5biNH8PAOrwJYziu1WP8h3hH66UqkqqTVhaYbHqoodvCAj3GUJW5hWk0DOrq"
      "miMP0LNmo2v46XEQGLzqsjBTC/smL/"
      "6Vf8Kyk4ygImxug1mCJ35OEsFnsJJmxpWeiOg06BbU8hDGJ5wvnIIPMgbVX3zr4qPBvY"
      "9Gd36Q7wujlUlUXrCnczq/Sn5K7vhjpOKPya6pxErSvMgmq1HihKQ/ePHXD/"
      "9piwO4fIlJ3H8eGLuihepnJmygyiqaYTccLSxqJzNXHalIPQjQLnIjRFyokqaBUKvPPZ"
      "ACDwoc/ovyrSrGPqmKpuMTPNsijx03gK7zBy4zhHXXhVkPz7q2/"
      "T1qFJ+UxzN0+4rmG8XHNTGajC4zeHYmOU4NVd2b4sEdyx4FUj/"
      "Z2f5fAfV7TELw64ZjP5MEQGFyHpxLmYjodcIObUqtR7tfk7t2XrnqH2jyx/"
      "RbsyW2EU7zzOCJ4+"
      "wrJqpN0QDWMg457Ml2TTPwtCGvU1d4KmZegthGdQwFuEBadl4bkvbmm8ED3aORT2Fhrm"
      "+ZsaMQGa2ybwmt5RebHlBail/"
      "PMOd2eKk1fRP8B0qSXdg4gY811fstYMo8karzgnSVziYQFDHU1jV5Doy7GYcVubYedVz"
      "PnryT25KEfI/"
      "38Htb4lKcPIJmpSi2p8RvxFbyR1HObSagYnL4UevOD7+b4Ayed9YhtPo/"
      "tfkQd1qn7OYl/dbm0KQWXqYxKS5dUzvn1BT1EAo+i5TYnH/"
      "A8+f1XlM7rbR0+zgX+"
      "P0Rw3tKq3IKr2QqODvI8rxc4BBK636VYfqDVh0smooDqaOWk0gSO6idUwOc2UjuEOdWB"
      "510VC45I8LjzKatplXp9+"
      "dXRjM3AsQGcFwnaQZvGIjcbRDRguN4dZZhGiwcdYgVHxxOdV9ARTZ+aRwnz/"
      "8AjNbEVuKdKArW2/fN+TH1q0hxc6W6ZPtLOwTC4m23hqkI0ge4ySu6jJQcXIaOtI/"
      "ATfCQ3aLWUv1cZAQxKq6P1r0o1Fy+Kct25Ww98xceFG16QDbH+"
      "v4PCRlYupBSZwEKFpT+JHia5CabdwC+Ljutw/7zbeF5Ee95/"
      "nl7EFWzR+BWXTapXVpNjBbtw+R2gf4gAQd+7fF4W/"
      "aM1dgdZBU6eVAJJOqkVKKEND6vtudW23XAWhCP009/"
      "H5fZXMnCYdiFVnjXi3rFdELy6PSdRyT1c8SC3Zw3N/fizLc/KFMqHTbhcxZnxkVx1/"
      "0DkYRZXeR4kW4z3EaO/"
      "OAmIwe1YXEHeZuSGss4iZ50bkYJ0zmsqlWiHBAhUmVLOxdTllleJvF8aeFGdnM4gam/"
      "QLAghWZUX0ePMdUAE5U5NfFEVH1g+"
      "KI4bgTB90HvxHF0L5vUD6dnJ2ynNDzwDnbALm0ldBx2jwuwmDuTEznlh53PDclfTTfgD"
      "aV2BrJmvmojLZESNWthUIf9IMijv2CI85wAMkN1+vf/"
      "UT1QmoyCvVl4TLeBqWmrebj3K5udX/aS8hjN1qjkv7HgzTZs0Q5rRIx/"
      "CmBQ4kJ6QJ8V45j743nU55rkTMWLdjIoAUDaYtrM26CCW1FHnefsRuMX7wBQ6qXP4Vzb"
      "7L10tmC11UWBsG8vI4YGmecvEgUGgE4w8cmsnGe+n/"
      "EtoM4wVQ5qlbVIqr3ikuy6fFp5dgeM8wq+"
      "JfptibAfPuPj9YFGJ1NdKspQcfuf16v85P3iPLb82eaujeDMRppaulKgv+"
      "G1Xkeg80nteQEQhk5e5IZCB90WG63sp5rMRfLed++"
      "JrdQJa47PIDTJQgIp4tM4VKPaVCMHpI/"
      "i4yFMFhDxEdDYLCXlixqccvaQXd1WBFRziBn7EFvFqvPI/"
      "3dk2zn3eyv3c1fS+IvnocvRnCi9gbbjTcZNJZ+kBEnGBWlB+X+uKeZ5AmZhyzR+d2dD/"
      "p2aOiEuDCmLxhlvbDJxsaSxFXpfA6I81e/"
      "gA5ZbL88tnvjP4KizhdzOpAzUsFMiP+MHtMMFp0FUeoDMV7DDS9Ty6CZjP6nix/"
      "WdJKEKPJ1CYWlCLFGztmRRrdD6ffSO/"
      "ac0QSy1dVUOvQSn8GTyhmrAqKEog25p2fh10EPnSjxxgIVMjT6F1FLGbJZuS9MdXEqA2"
      "r0JmOHhS0l2s9PJsYUnquw64syevsy1AozRvsitqFF3Vth4O/E+qJ5Mop/"
      "Os19LEYsL8PxmoZUCpt5cDXklYSgW/"
      "sdPj2ZHZiw5ojnKc5p7v0qbnom31KinCd8NStqhNoCeRop7cCuvVWyed42I+"
      "ykCslnYNsTiP5BWQesc/ME2FPMfQs5rfc1zcN8GrDOQuMeBiyp7PQF/"
      "jAd+1vaKGOkQtYdePP/8efkEPvMA/"
      "+ayifl3yg1TzIm3IgYQ+jyU4GnH3e92a4AsE2MzIarqve0/"
      "nk9mGrQ4t4zdrubhLElC7TckNGPtHpMpuFXtusdEmuwBTWy+"
      "NDtO7XbIGf2kIaNO4w13MTGQTvyzVNIduwqkuvSkFBVi6evkvZrflv1ikpky4PqsTZCb"
      "ubWfzdOaWSLkn9gLdWHHvi3lhASpUzqzUNNo4TVPg6PkPijPMiP4oBplVV+"
      "q2MSIII2b7vAN0G3DouAc6NJ/"
      "oKpXSzY3tdjfJz2VeJy9YLbiC4YUpMhO7WL5jj+8v4M+Amo5qiJ9652pBn5a27x9lbj+"
      "GyfxWc39BZmgd7+t9dXlhed7Ff4iMtrNFWxwhIjq340Tr/"
      "IpMGfuxdiegYDPKGtZ0qYoIKZH5O0LSWRdwMg/enxq2RT/"
      "PNYLkiyybcm1JnPE+MJwsMHqPHnHZz+TIMMZXthZk9nBqNWIrItgVQp8ZgPyWFEF5N++"
      "dHGCdzInukKuHLeVeE0OG+ADv0ASECpx2VK5ImlU/"
      "CkEdB6YsqKZKNXnN7lQsi3UcwEbaV4RuzRVDLl1qQ11XRj1TfQ3ux1+d2K/"
      "wySijKSr6dd4nzTyAEKaDymHPiy9L5/"
      "S6sITZKAOs2WpFxh8u1OCBtNN9CL2QhUJXKmkSwRMBDoYWj+"
      "GL8xpqDKK1l91GPOTzJp+djB8b+gJn6obMvbhhpj/"
      "pCV9j9P6B1EQHmR5hWM3RKYqiDQuwcvsJvPtZXmQoCLcoyGNaOPB4lMPAk5YBn0pcyWP"
      "OBIuEriFVmzDoHXXdVLaRU20SHBgA28TchPQ8q5quIUYdhqSQndH1S8ajR214Qa8Sv3N"
      "Gko8hBIBeqB2mrpokla+"
      "5XyNHcTwcl3IrBENGfHMmhJZx9AJHgf34TPNzs7bHKKbAdgAiywyH3TH9PWUJX9I0nQl"
      "gRx39kEc3u399/FBwGS5ALnsHD5WOds17XPyNzXWPpk3DNqG26t1D+Uhw7k/"
      "2B98trmzUYPOrqpLD6hOMj8N18MPBaGAPcdM1KqjZ+/tWLHFpti+uqULF1SVX/"
      "XCwMAVC61B24KkrVhSVXHCSdYGHU2omcUXniESRy55BfffupJ+"
      "z1B7Rz7iANJlmQHeWVPhqGfhBelIcZDgYcfKfjm5kCvQvtTDh0NjV4nV1TAJ8QWwzgVa"
      "wZKBl10gxWeZJyCxeTe3/"
      "RwV829NE9ehDdIX7ywTh3MyvMjeAf+9fjafZ2knOS5GDnP8RcYsyGo9uu/"
      "QTrNjIVB5QXQ/QPwANX0wLJ5/"
      "kGIUZ1KW2x0IIAXCPt9KqIh3r6domeaD0iU+N4xTfrmz6/"
      "3sVmdhhl9Ge22qMxJSBBmHW1/Oz4zMW2kydLImCKDTe13GqETeMEKdvuwJuE8/"
      "1tdtjJEUzHXNiN8XsVn1bMczXkfgKJHVUM6w4eLsaRr9HFtd45WUiDXVs2fH4BLA5Lx0"
      "9UhRj9kzMINCOkcRMOMc20yG9+jfTSAfF0lw3X0JgAGBEEIeXf8K/"
      "9S9z4WWb1xgPji+FGDNgSaoq3UWKSI+a26Lvy7AydVPBdmkJ45j2WSoFzpyk/"
      "qIFpIGQckJNIL7LnKTIoukDUJTgm90gIIPrun55oWFG96mwQ91EhiIp/"
      "g6AnvU9WQchSJ+r1qgs3vxRFIHqEVXFjlCznGSTTBEfXNJXDnCq9VM6yoXyOcJbdn7/"
      "agsaqLsbbfOqql2NM6ZEgxbPlUzZBrwsqGdQzXfx5Q/"
      "mcOpJNOuYtf3XpZhPNw8dq+eM80x59pvR+SirtfnsrE6MtsZAd1gwCDExAdVSWLorM/"
      "98CxAilYEMGEs9rgML1IKaGnkv74tVlteNJdzeL2XDoT6+fk8fZczD/"
      "yHijs8rjBAgRwPVQT3wURmNvP1o5SVP5L+hOjRnI+uGOl4lUaiS7b1+"
      "GwZ08nMLZDQjRNnR8Gc/50AGd4WOXv8tGJpe/"
      "x8zZAAZxuD6Z1b9dXGhaksqvBf5uUHGVel+"
      "Y2md4fpi0MyKCHxsnjKuciNiDmvot5xUsWe2BenZBm5rU+brmVvHoJ/"
      "ICUVG738XlgcY7yukmvOmINizB/kmNeio6q99MB/"
      "WiEPigP384Y38UQ7F6rCWL8rZ2KiTrnWUDFQrO0VfJDUOlfaMkZYyXdXQz/R/"
      "d2W9UjSPr8OBtNmPsrJ9DZ2a9VQDTwxghL9cyWr7BYziBf8PQs1nZ5fpu3kjtFaZ95L7"
      "qF7vPD8/TaP4D+W6JSPk4WsGNRZlNAtiU6Qnh+Ds60lObP9RJM/"
      "Lg1aRwpdlckvqg+3YGx6NnLTIPpD2FPWz5CuQiJ1wua+XjBbgl+"
      "WBYYa4qbINMxQIcxCVxDjLW8uj9j6O/"
      "t0crEeYJEv+SlGclZ9Jd3ViYQroaulmHmd6TY7q0dgxWnJ1+kMgzIklnIuk2Q/"
      "5ZOFsunj4dmXTeVOyaCwBY4H2UeavL/"
      "ejC+fQcKySidVwE4nowUDuQAIRzwarerFmKIYSs+3NQIoPoJukndP7pEQMefVW/"
      "ZG61HMT3H2/uyQnKQu/gWiO7jmBl/zwTzm/"
      "jnyM6Wq63dDZqidhrom7VoGApq7JQplxTIQaKHi1akjJ0adakFd70vMTnDp6sIerczJA"
      "fJsYkZhnhSlat+BOXFs0guNBY+Hd7KsVAHy7rexJ/wkvFmxYUSIvWLu6xgVB+/"
      "ws1zd7LnYLyYM52065itj6znjcUOto9XwTXtB0aCmpAH/"
      "NBanwiLDTRy4UOq92Br5Wz5N1fpsIL9lQeHkFIJ/nUZzqoh/6ilqvGWbXep5/"
      "Zf56wx2TGz7U8VgZ0FWyY1uHm7n3i9XH1LLqW1eidgxZRqqCV5bOipZr0cHsMvJoxGWM"
      "LJSJOoZg9tumR4nrykuQM3fHKPvHnXugnZZpWuveBsHN/"
      "yncze7sPMcJvFIQZMqfyLWpWf4B9eFJ0xFuViaga5IcLBLWJmHZNueLwtSEGkRsB9dzf"
      "oAn62sWwAM7/"
      "sLuzlhc8fRo6zI01pTwFcaVMpYZKeBDJplzFgLxghgK241rLDWOGsYGO/"
      "aPEg+K4TyX+BmWZIuGoYF591FDb/HbRcs5kyvbXsxpF1R/"
      "mBdP3zTPoKbJvYVyT3eyPjnzWxesOQxc6HPtyHF3eQ9z+gxJa6sZj9riC4/"
      "EOsAXSLc3DxB11lFyM6y4xRXfwYY3AhThfd4s15Dr4QMVJ007cXndgtPWD7udeAB7hvb"
      "ClUVt3CLbus4KQ3Jd+"
      "4VuHLGhYZJ8jQgjTOILSnMGV4BgnctrvscBiAk17QLdX1kpWL9VTjrAjpassniScpJ6C"
      "QFtzpjxCO3yucC++"
      "hADAw3fMGFWpA22L3U4K7UxWSHxp9pSX7CCGoslY5QsJJtQ4TydbIjRFHMvYgt3hd+"
      "sQ42+eaoqFflYbg5Q46iBalLRIYUv5NhbaZIu65TXq/"
      "1o1O0O1G9tye8DnD9yvRRGSSDXzaqksODU4gAFv1yU3LvFIy5VxWZzE2RotO5DbpmVzu"
      "+YMiILksbGncoHtpkmo5q8a2RbCd9QNzbXAX+COABQe1tNlE0gRLWr1r0BRdZ/"
      "L16SjhH5PDiWQkvCO6+mJAgD5JxNR9YG4CG/7tNHVcMLtIgpNqXxdBqVKhy/"
      "tcPLN5sPcYywCSu+Fe+zuueRSfqj7xp46VOffCAgA9B3/"
      "aWeYSfsnbtVBFWsuC0uVWjKPn7e/QCHK/"
      "BGWLyHmSbFxJXZ938I3QFn1YVZ5estajJfxlCRcE9YvrkUaAFP17NYaS6dD05H+"
      "tqqr8hWp937CaSdQUVz05wkkhkUUP/"
      "J5iVUKQp1ulJJpI3QGjRRBTsj3VJP7lzKAARbNJN94hf8npnu1dP13oUsw5W+"
      "zpVzvmAdEV6TgEeeZIyUvJhBEgQLRTsvuTH74uXOhFJdYsXOCT0YmE/"
      "LPCyz908UnVz8Wlwg5MswUXsPd1WubCXEL/ha3Cc9Fvpd1oZmo/"
      "tMvMquFVQxmS7tdLhYX0YW6aGaJINNnuaLSY4YSt3PH584IZEbGpMBRLK8tHWsW/uG/"
      "Km6h5+RYXPYPUnODuGE76WKME22Mbkkr5Q+F8C8Mx5fwJQdct+BnC9TP0xFZZio/"
      "Sm7VxVKXteeGn1ZnYCiMRdmFr5Z4D4Z23cBRoXOD2AggM+"
      "GM7S9urxRFy9klZn5B2SyPMAjFKCfGuvxYbCPTnHl2pUOssPn+"
      "uLpNt06ecgzHsXOancpaIvAGZfFir2MVADbE8UZ+27EEGtFMJaxld0kgq6VeP/"
      "XG8DIzx6xjvzhDECBJ12IamQqXi5tjUkVYmKm0svMYNNLoj8yVSfSRIrH92UUn6jdAG/"
      "vfa1x/"
      "3xhSGsaYUs6XKWqx+3JIcTjaXSBdXoAzKSa90tp+"
      "vH49wz3CIJliIDxdX5VJWB92FdQKCChDu2ZObD6zKIqldOZcbfKErtVHTiyarEiLMnEL"
      "pXXqhz7AKDEoFL4UVlKY8OLdwGIEXdKU0cjYjL2jEqRm4clSz4mkgv+"
      "sdWzYEMcQY19WD11ajp0rNVZJeTmaD9Qkc+"
      "2I2cNPFsF0NKDsKph0oOvpxvRiRP93h62g05yROb48XEXOl9+0BgndOpXnNruqjwUjU+"
      "LfO2v+zYmwrnevFxEbTqVRS+"
      "Ip5uWc5IHoL7IEYxAYYxAyxCfoUYF5cW9LXcUa3G9EgKO+YS+h/04yAREtL/"
      "8KtIvhgEOXE6gYjCP/"
      "Q8o2l2yT3PCDQiWfjaXlqSQK2HU1t7x64mo+hs4PT+VDWCh+"
      "RXLKP7XLBdoBBeLa6RHgH1128Y+xNxtCNzqdfRx9MG4R5hxCe6/"
      "1zCzjrNdxwnDSTuBt84d4vvo0jEapUdfQpKuJzbom8kW023TvWUcZIXHMzaJQybII6sW"
      "XGrYvGcrcLxNl1X+IQwPKGywWxPWp35jIhIiFtm6Xw0EC+8z/PVrI5/XYbT9kq4DH8C/"
      "/LoPwnzkfsEGUC111Y5pcbZFbeFELoTWa0jS5nDdXY8uWxzYkUVQxRgjAWPu70ycnMDR"
      "Jqg29Pzatt4guCUXh/lpbNECqtIsxeBa36cWadx2mhNuMvGBtC/"
      "NZbmeYaFtmGnLYZrDz58BOVrJiENNK6CAoWIHaAFxtoBhgF5FSsE9hV8+"
      "Z9H6WPDyRSKEO9OVkOsbrnJHkLB7oT1I+WwKT8EKVCQKraXua7ti+"
      "A3Kh5s6XBgUwyzk5B0d2nEzl5x+EV4Lx4HzONfUXHv56u4cObD+"
      "MRaO7BtDjj1e5oD6a/QdDU7AX5CbjPSAomTUwX/"
      "9KkwX5KL1n6RKaCTvuDpeciOPOinErKSnkyeGgDuyoWkfYy8eJgTaf6AHkQBm6gqzu80"
      "zPbcXkBCGLUKZncPibmC8Ty1anN/2BQgrYnrTQhd6ypN9CM+03E/"
      "BwWoAXkvmSKN4NAoawv/+vv+6tJPofI5Y0Hu8utDFf7/"
      "TjNaPMYCG26PtE+uQJkEICbSBNmBIdvbOWo34dF0276BGJ5yrl/gccfhKegMJcYQ/"
      "3R7mcknww0WuFf/"
      "6rJlDJ+dwWpUxC8oJM9RrgENRpDZp1Nrc8rAXl1r1GliwxdnQJkio1rnGXVTeLaE8FC/"
      "EcE5+Wi847ECdj+"
      "lTLYdK3T7z4uuiGBz1SwVC28GhH5KknsZLWLC9QCSALa9bRelkxDIKknfZC3IU/"
      "CEkHvENURq4W5s6ATONlOh9luuESUEazhT5JBGXfJZ3Tx4nd1ds0l3srALniPBdMZOxx"
      "VFtJ8juPL7FMIbUH0P5vltMIKwYhjAgaKhy3SNDnTH4AfsiDt3k501WeQlrvCzG4La7v"
      "qJFShLrHDtCjoJv0d+OmMnQ47v4OBkONtU+o/NBhe5IUufJ82zWARz7rh/i/opfXy/"
      "butpKBrxfcQpZ2K/"
      "BVUu9CSteWHkZ2vzODIj0DNtdUlV5Cx52pep0vAlN7FhX41fVowiRytM8kyQlVC+"
      "5akMBgLaPuDDcwZcz8Ly/"
      "fPlWuBZ4d6xEhjgRyte7QPtzqtYm5UQdDpqLy8WXZ0cMgSrJPjnkknW9FDjAZGMWCZ3c"
      "Ld9n48pcnDgStwCx6cQ0w1JNczF2dKrf5HpQL2S+Wt+foF3RQNXsCm+"
      "rpmqtNLP8MVanvXAkNBZN7nNe87i3EedcvNQs0/"
      "oqxB0SE5tVjYDrt1fK6sC5cCF3zIMAjsyxvUb/"
      "s+iytB2MqRoLd3WBxohptsZrcmrskFFch6X1U+"
      "11SMLziupGopt4m9jSoxlaKBYDj5T2FFfoSc2JGmNaL7uIOo/4m404AMrlDdCibQkV+/"
      "98do7tZBFzGz9gtLeS5zMcA0jTnuWWIuGDVsVLdVDbx+T8GJeY9RrxO/YhtN/"
      "PZae2+MuZZ+fKCiByK2MnbziwmtkgTN/W/"
      "LT32DzK3t61581XtrS8XWHBKHDvu1fntw+DD73PjGL0SLg+bcGNgD3z/12Uul+Ivi/"
      "78ZTjcDBUah7zV6BhTgCiJwBiBNAXyScOwi8H2AjpgxBsb2nqLBAeCNTmwEzXHseiuG2"
      "d2beEZkZJDyiRn3hwGhruxktB784BT6FUQ9iDIV9sR+"
      "kul0e2OMSLc017gFF1mfVrCNQciYm1VniSLqQaUIdemS9hI8pyyI3f1vB15Blgs12EEJ"
      "zeQQqixzmDiymw26a86Kkn7yhdSin51ocKi09TsnNG33YFK/"
      "hMfN793PrqOxeG5g+nSLi3NoQjQltCH6VSAMrKxBirbBYBStnxrunn9gAq8/"
      "GExdq+pzp1TKPNBtEibU9Lluka23KbM0NWk4amlZvMFitbXzXXWS+"
      "GFqBWqQAeZpc41ajcXq53/"
      "59sZPd3YVo2xBH+MuYYDAx50R+"
      "ec3S36zw6cS7nHguh5Tw7oGvXCZciTs2RZgq9fuawnci8X1fJYUe3K8qrXweqggAyZl4"
      "+v4Zy1xavPm0+kSaQsN2jarZ95NykWqMjq4cE44RxVEv48gX5+"
      "UILZE4fdLnuZWFnQC70LYQL9SNkPwSOS09jc3bfeHcVgw+kBskyK+JcRweG3+"
      "JK7nohbyRGxmqtoffcNgUghAAPnv8ckNb1G+"
      "2yPY8BuGOw3evaaZ5w1G8vJxjJGf5sgvaQjA1GIJfj4xmIWYDSJkwCgg5AGaH3g592rh"
      "ONpydIi1w/+Jx//gae62vFfpGvSf9ttg0iddLcLLoNijB2IlcjklBYfaa6/"
      "p00h1oO44P3iQTjTXRCda27Yd7SBlUcHcnkgkHZRIeECbyOVVHbHLbanSkg2MXBUjihI"
      "v1h0xXh7e3YCls8+CHJU4fwE6jelCBZgYE/"
      "+TLBnOxvO1AX8K696a4FOnBjjRowPqP7UeJfBh1V11IPIm74tO5Gkrez6x4QCLKvbvMC"
      "+6fqjCWRNkRuu8BdIw8VLK+EHQMJt288ZqdeQG9ybqaSDIpIpTTPQZNWR/MQ/"
      "HR+xpuRo0I441HaJKD281hPhBE2HTCGe57haxfpoPts0FkjjGt0SDCnXeB5L1k2YCa/"
      "eqyZwnNl+ftMoHstX2fdD+BnGZV/o9nCHf788gO0oU/"
      "nfAsTqtldUPDg+"
      "aj4O88m8XgVwzfq5HcKb9RGTVtxrhJxsZSkIQaB1bXPByXHqTimZ64hScXzQifOKZ9t1"
      "bw+FJKeADzCNzjKkLQK0gS0uhJgEZv10tILiBSSV6vstLO2FPoCOIx8hu+"
      "qwZlmnlD9kotPKOV1JjNFh6x3dq1KEX8NOa1xnFun78MSnNhbiu9DeRRvBBu6XFVQZ/"
      "+f9T4ZklRwVbFIwQ7RXPcfFOfkizo4pTmjihkt/"
      "MntRYZYMcgnBFq7tbJIVl5EtgQ7kmhiyHfl8Q3lrL10Mc3tUAgRmhdXRdEofV8GQQ8Ll"
      "60NHV2W5vCyJw7r53P00GWXx20JaX983RXznH7ImYajBTC+0VRQHjHp/"
      "A9moKQ3ejbNQ2qc3D9gDZjZiIeNjqEn4Lk0tSTMI0yZ9w/"
      "FAG8Jl38oejNXConPTPVfWhFWdxfIkKEbEhXpsSR7+8iP02ZsPFKu9BW+"
      "MmUVhqTxLmgMAPQPPDuIURR9Z+"
      "fQ07yX0IGGIDVz3l761W95qHH7EwPz4J1dpLdLir2wRlYI2kOMgEX6j2RYC42ThDX+"
      "2SGp+A6FHhrGdDYTB2BSt1cI9rZaEKRdjpA6OXBmv1QX4+"
      "PNXHySfNdovVh8077OcburwZ36C595eekPEo3rSlVHVFpq5q96tlKhdIa+"
      "RavTZZiJaH808y0eX7xXLEOcBlVQ93j6WyPJptIxASCaHRSQDcYknuHEleLZqPz5Y+"
      "3Y1PVmVRFgJ1TC7lVH9rzw+6/"
      "tRMs54VaQ+"
      "7I0xc2bs8A7RvKYGyOScFqGxBos3RgvUyoarSJ1o7kpp3d3Cu5M47OHsVES3bLCZ1BBs"
      "J0K4XwwAz2BHwHsvXMc+/"
      "8hQ+"
      "lKVj9yOiyRMz8FrK1ErHFqIuvD6QVsOb56Kz8EADR6HGVzidSmaAQ9rHdNarDsAR0pmR"
      "yStC2WI74tcDRy7q5vceV7goNZlpoLXQ4rxzJBdIANbsFcRYIvb34G6nAcGkXVGe/ut/"
      "4pNxFhzgVjTgxVMckqnIVvut6iL8J77cQ5J/q4S286QlAoDFgjj72mv8naB14mu0Yt/"
      "sppkPrvMnaSv1xNVTEdHz88zE6XupYGmt73yEGsrjbYLmSJEcDqy1H5SY3GbCJCzr0JO"
      "DNZ5HZUR37OS3PKAvpbXXGreCMxcA8+"
      "xhhsV1q69wI3KR51Vh0FBbSDaeKc2vt6xcviPpOVwffxaGMSfl2KMfHrpORIbzIBgVXG"
      "ihrCYpfVfSDa7BATqhZg66w0YONxHDaKFsuaHwYJ3st6TAgxrxcO7BUaQr2qxXcZqtCq"
      "W7mY/"
      "ixxXKRAR42xBmvyyBETQkle8lbJ0eqE6r0SnCSv7+"
      "0boohoiivUFOEG460d3PBEHxDWtwqLwJyyeYDMolQvCulmO+"
      "X8K778X2M1ZBLjHwc4vyelsCfchBfaRLGzGv3vO8dUigQsKxHDKKT4XOroNMaxzvBCTO"
      "vD251l8mNJUlxSEMd3SQoOE8XQ1oERvOINq5qZWlLMoCiakTyWLoDdFR2hFeDF+"
      "26jx6tRG9GcIDhkgL5bylApiP1mrbPgO9uWKNdYdHtWEsrE4i/"
      "9ehRhMTtIR0y3n7SLm+nLxbxmRQoy1HKdTpmV3c/"
      "TRe0mROi0Cv642PrVjgsrGmIZZ7NjHA3vcZltbE5mLw41gat7yESLaaFO/"
      "4mFIWf2lEJ9bvlMJGMB84hVynDQNJ6QN528GCS0ji+"
      "IEup30GIJQvnZI1gC9gRHDflQnWDGV+"
      "EzxDVnP0JbTnqbaHBisQhhEWiuHeYzRvrJQdmmf1UoxqlRQft88qD8e+"
      "Mig6uTIKjK8lDaHMpGcB9jgUYXcBNOKlRDoFG3BdxSjL4w5QEssXnW4iImWRhfULkF87"
      "GaBpIV9Y+thHBE3djAkipxEBV/"
      "GiTOgl+"
      "lFe2CQDeVuiK7cCyAZvSg9Q9tZUSkhNmAuSqJpGOhm1CZmVxpS8uff9PLVve6FCX0DG2"
      "F/"
      "DoFPEJvrWGQm46erZHmF7QTcGDxn7b3dpnXsAl0yyaMw21CldwDj7oo0e9pMAdwd0BOA"
      "zDh+vkX9qUHYUk+e2/pTLxDi+/WBwjS+/"
      "bRk6+qDDYaQqS+zL59MI1Y7EDCLkVXWONZgvCnEO8D4/"
      "BlKAbfoNJEiBwIy6y8TLBQ5C1z4U4ZAN+AzFKh8Mh2gM6VTIiOdkXO/"
      "o0uMz2MQD9hC+TSmtyT2Gjme1vdOxbQPp7SBLaWjqn4+PvSz6QbOW1e0F1ABtu2+"
      "O11p2NEBWCsqxfzoZ1ymwDPKUCsg9KL84xR1OsXh4JUBhMVbi07+iH7/nQkQt/"
      "o1K8P9dxtLv1+mRe1pRzTj341eiyHOPe0OlZeyIsekcuFzleplTm/"
      "WjlW7CEm87OotAHq8Vf4CkpAp6BeHsCz+Dhaa37m04Jvt8Vs2CysnHGoit+"
      "Dam5tThcWr2gvKkX4YZk1GyznO7yruKuk81GTpBtFGpH5agb2+3D7+UElB/"
      "xph0pOrqR77jVC+QkVCcmkybTruvXD4d/beSws92Zx+uo/ffXQ/"
      "w17FwG+uvqJGs640YKIfyHx/WvpVMbjQHzsX0hGuoPMd8BkYD/"
      "7s94UlNNEIDz4FU7jxfrgbQmhOFwd5/"
      "xIcwAvNnCuP++ArKhDsiNNnzz0B9jVxnt8TSGEpIz1Iml0QSWylyqVf87eU+"
      "NGGyF2OYQVBWKpt2DSRumOLb0ZT/"
      "WLIkBfTyZ100Rc9LXNeMDoafkOO+R2Yx4EoCKMEdpqyXC4q/5ciAuksR0yH/"
      "3VbKbQVZj8ppIov+VsaMhkCrAlbY/"
      "xHasooomKW1O1zomykxKGpE0zpHe9RfLzYE+"
      "2kDkV6oNZBd1TXpspj8OJZ3q888DhS4XVcRKSl4xRJ6fzgOM0gqz4U3hh23weDmvRL61"
      "krBkZmW/SzhqoX3uUgQ9CWxKMDWKJQrTcqDz/"
      "bxolDNGV2qnQW7V6pK3rx+jztCtk7yRTFJx+2r61Jwd0rlhB9dQV80oZx3T+"
      "TOhjJHo7L+cb2+mY2e7jDVEQG/"
      "l04bw+MW4nE1bJbENTcgR7GXf5mGi1gJJt3kfOCWZADL04lelioEnz3+ZStlU4u+"
      "ZszkEYl49sE1vSC4vWbmkaF+pKPOmUSkdzn9TqSK1jOdibUaHxXbo9c4SvFTw5/"
      "+H6XKRGar0SjRWVbY3AlUj6fPcetAPWgJdEetWFOIjI7UpQNyATfxitxdkQFDQiZfihT"
      "wtrOzq1ntJKiKhfy3KQn4EO90x9xKrgxM08F5+"
      "5rpLh3Ei456PTrNlVLxf7ScJ1OPTZY8lMNEh5uRMeNm18fhc2iFmjdmL41WrwscAICa+"
      "Z5mzQ7FY4h1oCEwsCKUJoG3D51e5u1KCVHzGXCF8bDrfi/"
      "JWclTPCFv+lDoQQ1YchCO8USw6QbsCCdO3eRlfCxKc9rfFzzxeCzD+"
      "UU7Ugrf7JXvU6bjU9JEc4EGkYqsaNqIvD2wVibL8NIhESXTcafCvDkNVXREvVR61+"
      "7pR5ApSYNXhHEUh/wXNsvq0LMfEj63WWBX2ghJXJ4rAJ0zq3XQMr/"
      "kXwW0HrYRQFDFP6HeskQDwDncnO+VDcbrlyH2x67Q+2IwzLvPcORbFWVh6rmV6SEN+"
      "DTmb8T6JH2tHkSrt2g6of2pZc/"
      "+f0X6FQJdfQcqFHRHHGuhVXI1UIgj9QC78kRkXEtaOlxOOOObAYsq1zw3/"
      "vWqVun0PwOckJ3tuS+Jftl2dGFG0Yx4uV0op/JP8A9jog7m/"
      "ogjvf2BL4faAGd1bo99IU4jI/+CEl716Br2o8ZNZ1l5arMxKzcLf/"
      "9XgTNqT13hskgKy1dIAeVhrH9dFIafB9tHkgDXndtu/"
      "Z+IcNPPrfZnmGaRBXLHBfnwRgKYktYNfPGa/GKwC/"
      "4vxQdVtMIVfMbMMDVO9bBPA2834wlQrDaZ3I7oiLwdssCA+SXMH0EZ00rz1O74EOBl/"
      "O/uZPzFvph5oAaTwqrvsFFVEqajRZ9Ekq3MMa9lzRH9A==");

  REQUIRE(std::size(base64) == 15856);
  REQUIRE_NOTHROW(klib::aes_256_cbc_decrypt(
      base64, klib::sha_256_raw("913d8e1ebca5ef2193b4fea1fdbe0394"), iv));
}

TEST_CASE("folder_size", "[util]") {
  REQUIRE(std::filesystem::exists("folder1"));
  REQUIRE(klib::folder_size("folder1") == 38);
}

TEST_CASE("same_folder", "[util]") {
  REQUIRE(std::filesystem::exists("folder1"));
  REQUIRE(std::filesystem::exists("folder2"));

  REQUIRE(klib::same_folder("folder1", "folder2"));
}

TEST_CASE("execute_command", "[util]") {
  std::string command = "gcc -v";
  REQUIRE_NOTHROW(klib::execute_command(command));
}

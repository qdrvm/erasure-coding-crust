#include <chrono>
#include <cstdint>
#include <iostream>
#include <string_view>

#include "../ec-cpp/ec-cpp.hpp"

extern "C" {
#include <erasure_coding/erasure_coding.h>
}

static constexpr std::string_view test_data =
    "This is a test string. The purpose of it is not allow the evil forces to "
    "conquer the world!";
static constexpr uint64_t n_validators = 6ull;

class TicToc {
  std::chrono::time_point<std::chrono::high_resolution_clock> t_;

public:
  TicToc(std::string &&) = delete;
  TicToc(std::string const &) = delete;
  TicToc() { t_ = std::chrono::high_resolution_clock::now(); }

  auto toc() {
    auto prev = t_;
    t_ = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(t_ - prev);
  }

  ~TicToc() {}
};

void Cpp_Measures() {
  std::string_view test[]{
      "wasioghowerhqht87y450t984y1h5oh243ptgwfhyqa9wyf9 yu9y9r "
      "239y509y23trhr8247y p1qut59 2914tu520 t589u3t9y7u32w9ty 89qewy923u5 "
      "h4123hty t90y1982u95yu "
      "91259oy92y5tr90oweiovfdkljscnvkljasnhiewytr9q8uj5toinh1 "
      "wasioghowerhqht87y450t984y1h5oh243ptgwfhyqa9wyf9 yu9y9r "
      "239y509y23trhr8247y p1qut59 2914tu520 t589u3t9y7u32w9ty 89qewy923u5 "
      "h4123hty t90y1982u95yu "
      "91259oy92y5tr90oweiovfdkljscnvkljasnhiewytr9q8uj5toinh1 "
      "wasioghowerhqht87y450t984y1h5oh243ptgwfhyqa9wyf9 yu9y9r "
      "239y509y23trhr8247y p1qut59 2914tu520 t589u3t9y7u32w9ty 89qewy923u5 "
      "h4123hty t90y1982u95yu "
      "91259oy92y5tr90oweiovfdkljscnvkljasnhiewytr9q8uj5toinh1 "
      "wasioghowerhqht87y450t984y1h5oh243ptgwfhyqa9wyf9 yu9y9r "
      "239y509y23trhr8247y p1qut59 2914tu520 t589u3t9y7u32w9ty 89qewy923u5 "
      "h4123hty t90y1982u95yu "
      "91259oy92y5tr90oweiovfdkljscnvkljasnhiewytr9q8uj5toinh1 "
      "wasioghowerhqht87y450t984y1h5oh243ptgwfhyqa9wyf9 yu9y9r "
      "239y509y23trhr8247y p1qut59 2914tu520 t589u3t9y7u32w9ty 89qewy923u5 "
      "h4123hty t90y1982u95yu "
      "91259oy92y5tr90oweiovfdkljscnvkljasnhiewytr9q8uj5toinh1 "
      "wasioghowerhqht87y450t984y1h5oh243ptgwfhyqa9wyf9 yu9y9r "
      "239y509y23trhr8247y p1qut59 2914tu520 t589u3t9y7u32w9ty 89qewy923u5 "
      "h4123hty t90y1982u95yu "
      "91259oy92y5tr90oweiovfdkljscnvkljasnhiewytr9q8uj5toinh1 "
      "wasioghowerhqht87y450t984y1h5oh243ptgwfhyqa9wyf9 yu9y9r "
      "239y509y23trhr8247y p1qut59 2914tu520 t589u3t9y7u32w9ty 89qewy923u5 "
      "h4123hty t90y1982u95yu "
      "91259oy92y5tr90oweiovfdkljscnvkljasnhiewytr9q8uj5toinh1 "
      "wasioghowerhqht87y450t984y1h5oh243ptgwfhyqa9wyf9 yu9y9r "
      "239y509y23trhr8247y p1qut59 2914tu520 t589u3t9y7u32w9ty 89qewy923u5 "
      "h4123hty t90y1982u95yu "
      "91259oy92y5tr90oweiovfdkljscnvkljasnhiewytr9q8uj5toinh1 "
      "wasioghowerhqht87y450t984y1h5oh243ptgwfhyqa9wyf9 yu9y9r "
      "239y509y23trhr8247y p1qut59 2914tu520 t589u3t9y7u32w9ty 89qewy923u5 "
      "h4123hty t90y1982u95yu "
      "91259oy92y5tr90oweiovfdkljscnvkljasnhiewytr9q8uj5toinh1 "
      "wasioghowerhqht87y450t984y1h5oh243ptgwfhyqa9wyf9 yu9y9r "
      "239y509y23trhr8247y p1qut59 2914tu520 t589u3t9y7u32w9ty 89qewy923u5 "
      "h4123hty t90y1982u95yu "
      "91259oy92y5tr90oweiovfdkljscnvkljasnhiewytr9q8uj5toinh1 "
      "wasioghowerhqht87y450t984y1h5oh243ptgwfhyqa9wyf9 yu9y9r "
      "239y509y23trhr8247y p1qut59 2914tu520 t589u3t9y7u32w9ty 89qewy923u5 "
      "h4123hty t90y1982u95yu "
      "91259oy92y5tr90oweiovfdkljscnvkljasnhiewytr9q8uj5toinh1 "
      "wasioghowerhqht87y450t984y1h5oh243ptgwfhyqa9wyf9 yu9y9r "
      "239y509y23trhr8247y p1qut59 2914tu520 t589u3t9y7u32w9ty 89qewy923u5 "
      "h4123hty t90y1982u95yu "
      "91259oy92y5tr90oweiovfdkljscnvkljasnhiewytr9q8uj5toinh1 "
      "wasioghowerhqht87y450t984y1h5oh243ptgwfhyqa9wyf9 yu9y9r "
      "239y509y23trhr8247y p1qut59 2914tu520 t589u3t9y7u32w9ty 89qewy923u5 "
      "h4123hty t90y1982u95yu "
      "91259oy92y5tr90oweiovfdkljscnvkljasnhiewytr9q8uj5toinh1 "};

  constexpr size_t kMeasureCount = 100ull;

  {
    std::chrono::microseconds measured_enc{};
    std::chrono::microseconds measured_dec{};

    for (auto const &t : test) {
      for (size_t i = 0ull; i < kMeasureCount; ++i) {
        uint64_t enc_time, dec_time;
        DataBlock data{.array = (uint8_t *)t.data(), .length = t.size()};
        ECCR_Test_MeasurePerformance(&data, n_validators, &enc_time, &dec_time);

        measured_enc += std::chrono::microseconds(enc_time);
        measured_dec += std::chrono::microseconds(dec_time);
      }
    }
    std::cout << "Encode RUST (" << kMeasureCount
              << " cycles): " << measured_enc.count() << " us" << std::endl;
    std::cout << "Decode RUST (" << kMeasureCount
              << " cycles): " << measured_dec.count() << " us" << std::endl;
  }

  {
    std::chrono::microseconds measured_enc{};
    std::chrono::microseconds measured_dec{};
    for (auto const &t : test) {
      for (size_t i = 0ull; i < kMeasureCount; ++i) {
        auto enc_create_result = ec_cpp::create(n_validators);
        auto encoder = ec_cpp::resultGetValue(std::move(enc_create_result));
        std::vector<ec_cpp::ReedSolomon<ec_cpp::PolyEncoder_f2e16>::Shard>
            shards;
        ec_cpp::Result<std::vector<uint8_t>> decoded;
        { /// encode
          TicToc m;
          auto enc_result = encoder.encode(
              ec_cpp::Slice<uint8_t>((uint8_t *)t.data(), t.length()));
          measured_enc += m.toc();
          shards = ec_cpp::resultGetValue(std::move(enc_result));
        }
        { /// decode
          TicToc m;
          decoded = encoder.reconstruct(shards);
          measured_dec += m.toc();
        }
      }
    }
    std::cout << "Encode C++ (" << kMeasureCount
              << " cycles): " << measured_enc.count() << " us" << std::endl;
    std::cout << "Decode C++ (" << kMeasureCount
              << " cycles): " << measured_dec.count() << " us" << std::endl;
  }
}

int main() {
  Cpp_Measures();
  return 0;
}
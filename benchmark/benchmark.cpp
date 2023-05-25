#include <chrono>
#include <cstdint>
#include <iostream>
#include <string_view>

#include <ec-cpp/ec-cpp.hpp>

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
  std::string test_data[6];
  test_data[0].reserve(1ull);
  test_data[1].reserve(300ull);
  test_data[2].reserve(5ull * 1000ull);
  test_data[3].reserve(100ull * 1000ull);
  test_data[4].reserve(1ull * 1000ull * 1000ull);
  test_data[5].reserve(10ull * 1000ull * 1000ull);

  for (auto &t : test_data)
    for (size_t i = 0; i < t.capacity(); ++i)
      t += char(97 + (i % 24));

  constexpr size_t kMeasureCount = 100ull;
  auto printout = [&](std::string_view name, std::chrono::microseconds val) {
    if (val.count() > 5'000'000) {
      std::cout << name << " (" << kMeasureCount
                << " cycles): " << float(val.count()) / 1'000'000.0f << " s"
                << std::endl;
    } else if (val.count() > 5'000) {
      std::cout << name << " (" << kMeasureCount
                << " cycles): " << float(val.count()) / 1'000.0f << " ms"
                << std::endl;
    } else {
      std::cout << name << " (" << kMeasureCount << " cycles): " << val.count()
                << " us" << std::endl;
    }
  };

  for (auto const &t : test_data) {
    {
      std::chrono::microseconds measured_enc{};
      std::chrono::microseconds measured_dec{};

      for (size_t i = 0ull; i < kMeasureCount; ++i) {
        uint64_t enc_time, dec_time;
        DataBlock data{.array = (uint8_t *)t.data(), .length = t.size()};
        ECCR_Test_MeasurePerformance(&data, n_validators, &enc_time, &dec_time);

        measured_enc += std::chrono::microseconds(enc_time);
        measured_dec += std::chrono::microseconds(dec_time);
      }
      std::cout << "~~~ [ Benchmark case: " << t.size() << " bytes ] ~~~"
                << std::endl;
      printout("Encode RUST", measured_enc);
      printout("Decode RUST", measured_dec);
    }
    {
      std::chrono::microseconds measured_enc{};
      std::chrono::microseconds measured_dec{};
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
      printout("Encode C++", measured_enc);
      printout("Decode C++", measured_dec);
      std::cout << std::endl;
    }
  }
}

int main() {
  Cpp_Measures();
  return 0;
}
/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>

#include <ec-cpp/ec-cpp.hpp>
#include <ec-cpp/f2e16.hpp>
#include <ec-cpp/table_f2e16.hpp>

extern "C" {
#include <erasure_coding/erasure_coding.h>
}

static constexpr std::string_view test_data =
    "This is a test string. The purpose of it is not allow the evil forces to "
    "conquer the world!";
static constexpr uint64_t n_validators = 6ull;

inline size_t Prototype_log2(size_t x) {
  size_t o = 0ull;
  while (x > 1) {
    x = (x >> 1);
    o += 1;
  }
  return o;
}

auto createTestChunks(std::string_view input, ChunksList &out,
                      uint64_t validators = n_validators) {
  DataBlock data{.array = (uint8_t *)input.data(), .length = input.length()};
  return ECCR_obtain_chunks(validators, &data, &out);
}

auto createTestChunks(ChunksList &out, uint64_t validators = n_validators) {
  return createTestChunks(test_data, out, validators);
}

TEST(erasure_coding, Cpp_Encode) {
  std::string_view test[]{
      test_data,
      "wasioghowerhqht87y450t984y1h5oh243ptgwfhyqa9wyf9 yu9y9r "
      "239y509y23trhr8247y p1qut59 2914tu520 t589u3t9y7u32w9ty 89qewy923u5 "
      "h4123hty t90y1982u95yu "
      "91259oy92y5tr90oweiovfdkljscnvkljasnhiewytr9q8uj5toinh1 "
      "l;n4ou98uiqwp2j3mrtlknmeswlkjf p9o87q90p u2p45j243o56u9uyew98fuqw",
      "1"};

  for (auto const &t : test) {
    ChunksList out{};
    EXPECT_TRUE(createTestChunks(t, out).tag == NPRSResult_Tag::NPRS_RESULT_OK);

    auto enc_create_result = ec_cpp::create(n_validators);
    ASSERT_EQ(ec_cpp::resultHasError(enc_create_result), false);

    auto encoder = ec_cpp::resultGetValue(std::move(enc_create_result));
    auto enc_result =
        encoder.encode(ec_cpp::Slice<uint8_t>((uint8_t *)t.data(), t.length()));
    ASSERT_EQ(ec_cpp::resultHasError(enc_result), false);

    auto result_data = ec_cpp::resultGetValue(std::move(enc_result));
    ASSERT_EQ(result_data.size(), out.count);

    for (size_t i = 0; i < out.count; ++i) {
      auto const &chunk_ref = out.data[i];
      ASSERT_TRUE(chunk_ref.index < result_data.size());

      auto const &chunk_target = result_data[chunk_ref.index];
      ASSERT_EQ(chunk_ref.data.length, chunk_target.size());

      for (size_t j = 0; j < chunk_ref.data.length; ++j) {
        ASSERT_EQ(chunk_ref.data.array[j], chunk_target[j]);
      }
    }
  }
}

TEST(erasure_coding, Cpp_Decode) {
  std::string_view test[]{
      test_data,
      "wasioghowerhqht87y450t984y1h5oh243ptgwfhyqa9wyf9 yu9y9r "
      "239y509y23trhr8247y p1qut59 2914tu520 t589u3t9y7u32w9ty 89qewy923u5 "
      "h4123hty t90y1982u95yu "
      "91259oy92y5tr90oweiovfdkljscnvkljasnhiewytr9q8uj5toinh1 "
      "l;n4ou98uiqwp2j3mrtlknmeswlkjf p9o87q90p u2p45j243o56u9uyew98fuqw",
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
      "91259oy92y5tr90oweiovfdkljscnvkljasnhiewytr9q8uj5toinh1 ",
      "1"};

  for (auto const &t : test) {
    ChunksList out{};
    EXPECT_TRUE(createTestChunks(t, out).tag == NPRSResult_Tag::NPRS_RESULT_OK);

    auto enc_create_result = ec_cpp::create(n_validators);
    ASSERT_EQ(ec_cpp::resultHasError(enc_create_result), false);

    auto encoder = ec_cpp::resultGetValue(std::move(enc_create_result));
    auto enc_result =
        encoder.encode(ec_cpp::Slice<uint8_t>((uint8_t *)t.data(), t.length()));
    ASSERT_EQ(ec_cpp::resultHasError(enc_result), false);

    auto result_data = ec_cpp::resultGetValue(std::move(enc_result));
    ASSERT_EQ(result_data.size(), out.count);

    DataBlock data{};
    EXPECT_TRUE(ECCR_reconstruct(n_validators, &out, &data).tag ==
                NPRSResult_Tag::NPRS_RESULT_OK);

    auto decode_result = encoder.reconstruct(result_data);
    ASSERT_FALSE(ec_cpp::resultHasError(decode_result));

    auto decoded = ec_cpp::resultGetValue(std::move(decode_result));
    ASSERT_EQ(data.length, decoded.size());

    for (size_t i = 0; i < data.length; ++i) {
      ASSERT_EQ(data.array[i], decoded[i]);
    }
  }
}

TEST(erasure_coding, Cpp_AFFT_tables) {
  uint16_t src_0[65535];
  ECCR_AFFT_Table(&src_0);

  ec_cpp::f2e16_Descriptor desc_;
  ec_cpp::PolyEncoder_f2e16 init{desc_};
  auto src_1 =
      ec_cpp::AdditiveFFT<ec_cpp::f2e16_Descriptor>::initalize(desc_.kTables);

  ASSERT_EQ(sizeof(src_0) / sizeof(src_0[0]),
            sizeof(src_1.skews) / sizeof(src_1.skews[0]));
  for (size_t i = 0ull; i < sizeof(src_0) / sizeof(src_0[0]); ++i) {
    ASSERT_EQ(src_0[i], src_1.skews[i]);
  }
}

TEST(erasure_coding, Cpp_EltBEEncode) {
  uint8_t a_0[] = {0x11, 0x22};
  ASSERT_EQ(0x1122, ec_cpp::f2e16_Descriptor::fromBEBytes(a_0));
}

TEST(erasure_coding, Cpp_MathNextHighPow2) {
  ASSERT_EQ(1ull, ec_cpp::math::nextHighPowerOf2(0ull));
  ASSERT_EQ(4ull, ec_cpp::math::nextHighPowerOf2(4ull));
  ASSERT_EQ(8ull, ec_cpp::math::nextHighPowerOf2(5ull));
  ASSERT_EQ(8ull, ec_cpp::math::nextHighPowerOf2(7ull));
  ASSERT_EQ(8ull, ec_cpp::math::nextHighPowerOf2(8ull));
}

TEST(erasure_coding, Cpp_MathNextLowPow2) {
  ASSERT_EQ(1ull, ec_cpp::math::nextLowPowerOf2(0ull));
  ASSERT_EQ(4ull, ec_cpp::math::nextLowPowerOf2(4ull));
  ASSERT_EQ(4ull, ec_cpp::math::nextLowPowerOf2(5ull));
  ASSERT_EQ(4ull, ec_cpp::math::nextLowPowerOf2(7ull));
  ASSERT_EQ(8ull, ec_cpp::math::nextLowPowerOf2(8ull));
}

TEST(erasure_coding, Cpp_Polyf2e16) {
  ec_cpp::f2e16_Descriptor desc_;
  ec_cpp::PolyEncoder_f2e16 poly{desc_};
  auto &[log_table, exp_table, walsh_table] = desc_.kTables;

  ASSERT_EQ(log_table.size(), sizeof(LOG_TABLE) / sizeof(LOG_TABLE[0]));
  for (size_t i = 0; i < log_table.size(); ++i) {
    ASSERT_EQ(log_table[i], LOG_TABLE[i]);
  }

  ASSERT_EQ(exp_table.size(), sizeof(EXP_TABLE) / sizeof(EXP_TABLE[0]));
  for (size_t i = 0; i < exp_table.size(); ++i) {
    ASSERT_EQ(exp_table[i], EXP_TABLE[i]);
  }

  ASSERT_EQ(walsh_table.size(), sizeof(LOG_WALSH) / sizeof(LOG_WALSH[0]));
  for (size_t i = 0; i < walsh_table.size(); ++i) {
    ASSERT_EQ(walsh_table[i], LOG_WALSH[i]);
  }
}

TEST(erasure_coding, Cpp_Math_Log2) {
  ASSERT_EQ(Prototype_log2(std::numeric_limits<size_t>::max()),
            ec_cpp::math::log2(std::numeric_limits<size_t>::max()));
  for (size_t i = 0ull; i < 1000ull; ++i)
    ASSERT_EQ(Prototype_log2(i), ec_cpp::math::log2(i));
}

TEST(erasure_coding, Cpp_CreateChunks) {
  auto result = ec_cpp::create(8ull);
  ASSERT_EQ(ec_cpp::resultHasError(result), false);
}

TEST(erasure_coding, Cpp_RecoveryThreshold_Few) {
  auto result = ec_cpp::getRecoveryThreshold(1);
  ASSERT_TRUE(ec_cpp::resultHasError(result));
  ASSERT_EQ(ec_cpp::resultGetError(std::move(result)),
            ec_cpp::Error::kNotEnoughValidators);

  size_t out;
  ASSERT_EQ(NPRSResult_Tag::NPRS_RESULT_NOT_ENOUGH_VALIDATORS,
            ECCR_get_recovery_threshold(1, &out).tag);
}

TEST(erasure_coding, Cpp_RecoveryThreshold_5) {
  const size_t kCount = 5;
  auto result = ec_cpp::getRecoveryThreshold(kCount);
  ASSERT_FALSE(ec_cpp::resultHasError(result));

  size_t out;
  ASSERT_EQ(NPRSResult_Tag::NPRS_RESULT_OK,
            ECCR_get_recovery_threshold(kCount, &out).tag);
  ASSERT_EQ(out, ec_cpp::resultGetValue(std::move(result)));
}

TEST(erasure_coding, Cpp_RecoveryThreshold_100) {
  const size_t kCount = 100;
  auto result = ec_cpp::getRecoveryThreshold(kCount);
  ASSERT_FALSE(ec_cpp::resultHasError(result));

  size_t out;
  ASSERT_EQ(NPRSResult_Tag::NPRS_RESULT_OK,
            ECCR_get_recovery_threshold(kCount, &out).tag);
  ASSERT_EQ(out, ec_cpp::resultGetValue(std::move(result)));
}

TEST(erasure_coding, Cpp_RecoveryThreshold_90000) {
  const size_t kCount = 90000;
  auto result = ec_cpp::getRecoveryThreshold(kCount);
  ASSERT_TRUE(ec_cpp::resultHasError(result));
  ASSERT_EQ(ec_cpp::resultGetError(std::move(result)),
            ec_cpp::Error::kTooManyValidators);

  size_t out;
  ASSERT_EQ(NPRSResult_Tag::NPRS_RESULT_TOO_MANY_VALIDATORS,
            ECCR_get_recovery_threshold(kCount, &out).tag);
}

TEST(erasure_coding, CreateChunks) {
  ChunksList out{};
  EXPECT_TRUE(createTestChunks(out).tag == NPRSResult_Tag::NPRS_RESULT_OK);
  EXPECT_TRUE(out.count == n_validators);
  ECCR_deallocate_chunk_list(&out);
}

TEST(erasure_coding, CreateChunksMaxValidators) {
  ChunksList out{};
  EXPECT_TRUE(createTestChunks(out, 70000).tag ==
              NPRSResult_Tag::NPRS_RESULT_TOO_MANY_VALIDATORS);
}

TEST(erasure_coding, CreateChunksMinValidators) {
  ChunksList out{};
  EXPECT_TRUE(createTestChunks(out, 1).tag ==
              NPRSResult_Tag::NPRS_RESULT_NOT_ENOUGH_VALIDATORS);
}

TEST(erasure_coding, ReconstructChunksFromWholeData) {
  ChunksList chunks{};
  createTestChunks(chunks);

  DataBlock data{};
  EXPECT_TRUE(ECCR_reconstruct(n_validators, &chunks, &data).tag ==
              NPRSResult_Tag::NPRS_RESULT_OK);

  std::string_view result{(char *)data.array, data.length - 1};
  EXPECT_TRUE(result == test_data);

  ECCR_deallocate_chunk_list(&chunks);
  ECCR_deallocate_data_block(&data);
}

TEST(erasure_coding, Reconstruct1_3) {
  ChunksList chunks{};
  createTestChunks(chunks);

  std::vector<Chunk> tmp;
  for (size_t ix = 0; ix < n_validators / 3; ++ix) {
    tmp.push_back(chunks.data[ix]);
  }
  ChunksList chunks_2{.data = &tmp.front(), .count = tmp.size()};

  DataBlock data{};
  EXPECT_TRUE(ECCR_reconstruct(n_validators, &chunks_2, &data).tag ==
              NPRSResult_Tag::NPRS_RESULT_OK);

  std::string_view result{(char *)data.array, data.length - 1};
  EXPECT_TRUE(result == test_data);

  ECCR_deallocate_chunk_list(&chunks);
  ECCR_deallocate_data_block(&data);
}

TEST(erasure_coding, Cpp_Reconstruct1_3) {
  auto enc_create_result = ec_cpp::create(n_validators);
  ASSERT_EQ(ec_cpp::resultHasError(enc_create_result), false);

  auto encoder = ec_cpp::resultGetValue(std::move(enc_create_result));
  auto enc_result = encoder.encode(
      ec_cpp::Slice<uint8_t>((uint8_t *)test_data.data(), test_data.size()));
  ASSERT_FALSE(ec_cpp::resultHasError(enc_result));

  auto result_data = ec_cpp::resultGetValue(std::move(enc_result));
  for (size_t ix = 0; ix < n_validators / 3; ++ix)
    result_data[ix].clear();

  auto decode_result = encoder.reconstruct(result_data);
  ASSERT_FALSE(ec_cpp::resultHasError(decode_result));

  auto decoded = ec_cpp::resultGetValue(std::move(decode_result));
  ASSERT_EQ(test_data.size(), decoded.size() - 1ull);

  for (size_t i = 0; i < test_data.size(); ++i)
    ASSERT_EQ(test_data[i], decoded[i]);
}

TEST(erasure_coding, ReconstructLess1_3) {
  ChunksList chunks{};
  createTestChunks(chunks);

  std::vector<Chunk> tmp;
  for (size_t ix = 0; ix < (n_validators / 3) - 1; ++ix) {
    tmp.push_back(chunks.data[ix]);
  }
  ChunksList chunks_2{.data = &tmp.front(), .count = tmp.size()};

  DataBlock data{};
  EXPECT_TRUE(ECCR_reconstruct(n_validators, &chunks_2, &data).tag ==
              NPRSResult_Tag::NPRS_RESULT_NOT_ENOUGH_CHUNKS);

  ECCR_deallocate_chunk_list(&chunks);
}

TEST(erasure_coding, Cpp_ReconstructLess1_3) {
  auto enc_create_result = ec_cpp::create(n_validators);
  ASSERT_EQ(ec_cpp::resultHasError(enc_create_result), false);

  auto encoder = ec_cpp::resultGetValue(std::move(enc_create_result));
  auto enc_result = encoder.encode(
      ec_cpp::Slice<uint8_t>((uint8_t *)test_data.data(), test_data.size()));
  ASSERT_FALSE(ec_cpp::resultHasError(enc_result));

  auto result_data = ec_cpp::resultGetValue(std::move(enc_result));
  for (size_t ix = 0; ix < n_validators / 2 + 2ull; ++ix)
    result_data[ix].clear();

  auto decode_result = encoder.reconstruct(result_data);
  ASSERT_TRUE(ec_cpp::resultHasError(decode_result));
  ASSERT_EQ(ec_cpp::resultGetError(std::move(decode_result)),
            ec_cpp::Error::kNeedMoreShards);
}

TEST(erasure_coding, Cpp_Reconstruct1_3_Border) {
  auto enc_create_result = ec_cpp::create(n_validators);
  ASSERT_EQ(ec_cpp::resultHasError(enc_create_result), false);

  auto encoder = ec_cpp::resultGetValue(std::move(enc_create_result));
  auto enc_result = encoder.encode(
      ec_cpp::Slice<uint8_t>((uint8_t *)test_data.data(), test_data.size()));
  ASSERT_FALSE(ec_cpp::resultHasError(enc_result));

  auto result_data = ec_cpp::resultGetValue(std::move(enc_result));
  result_data[0].clear();
  result_data[1].clear();
  // result_data[2];
  result_data[3].clear();
  result_data[4].clear();
  // result_data[5];

  auto decode_result = encoder.reconstruct(result_data);
  ASSERT_FALSE(ec_cpp::resultHasError(decode_result));

  auto decoded = ec_cpp::resultGetValue(std::move(decode_result));
  ASSERT_EQ(test_data.size(), decoded.size() - 1ull);

  for (size_t i = 0; i < test_data.size(); ++i)
    ASSERT_EQ(test_data[i], decoded[i]);
}

TEST(erasure_coding, Reconstruct1_3_last_one) {
  ChunksList chunks{};
  createTestChunks(chunks);

  std::vector<Chunk> tmp;
  tmp.push_back(chunks.data[1ull]);
  tmp.push_back(chunks.data[5ull]);
  ChunksList chunks_2{.data = &tmp.front(), .count = tmp.size()};

  DataBlock data{};
  EXPECT_TRUE(ECCR_reconstruct(n_validators, &chunks_2, &data).tag ==
              NPRSResult_Tag::NPRS_RESULT_OK);

  std::string_view result{(char *)data.array, data.length - 1};
  EXPECT_TRUE(result == test_data);

  ECCR_deallocate_chunk_list(&chunks);
  ECCR_deallocate_data_block(&data);
}

TEST(erasure_coding, Reconstruct_WrongIndex) {
  ChunksList chunks{};
  createTestChunks(chunks);

  std::vector<Chunk> tmp;
  tmp.push_back(chunks.data[1ull]);
  tmp.push_back(chunks.data[5ull]);
  tmp[0ull].index = 3ull;

  ChunksList chunks_2{.data = &tmp.front(), .count = tmp.size()};

  DataBlock data{};
  EXPECT_TRUE(ECCR_reconstruct(n_validators, &chunks_2, &data).tag ==
              NPRSResult_Tag::NPRS_RESULT_OK);

  std::string_view result{(char *)data.array, data.length - 1};
  EXPECT_TRUE(result != test_data);

  ECCR_deallocate_chunk_list(&chunks);
  ECCR_deallocate_data_block(&data);
}

TEST(erasure_coding, Cpp_Decode_Big) {
  std::vector<uint8_t> test;
  test.resize(1ull * 1024ull * 1024ull);

  uint64_t counter = 0ull;
  for (auto &s : test)
    s = (++counter % 0xff);

  ChunksList out{};
  DataBlock data{.array = test.data(), .length = test.size()};
  EXPECT_TRUE(ECCR_obtain_chunks(n_validators, &data, &out).tag ==
              NPRSResult_Tag::NPRS_RESULT_OK);

  auto enc_create_result = ec_cpp::create(n_validators);
  ASSERT_EQ(ec_cpp::resultHasError(enc_create_result), false);

  auto encoder = ec_cpp::resultGetValue(std::move(enc_create_result));
  auto enc_result =
      encoder.encode(ec_cpp::Slice<uint8_t>(test.data(), test.size()));
  ASSERT_EQ(ec_cpp::resultHasError(enc_result), false);

  auto result_data = ec_cpp::resultGetValue(std::move(enc_result));
  ASSERT_EQ(result_data.size(), out.count);

  DataBlock data_out{};
  EXPECT_TRUE(ECCR_reconstruct(n_validators, &out, &data_out).tag ==
              NPRSResult_Tag::NPRS_RESULT_OK);

  auto decode_result = encoder.reconstruct(result_data);
  ASSERT_FALSE(ec_cpp::resultHasError(decode_result));

  auto decoded = ec_cpp::resultGetValue(std::move(decode_result));
  ASSERT_EQ(data_out.length, decoded.size());

  for (size_t i = 0; i < data_out.length; ++i) {
    ASSERT_EQ(data_out.array[i], decoded[i]);
  }
}

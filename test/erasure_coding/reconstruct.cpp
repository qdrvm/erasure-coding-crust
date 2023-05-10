/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>

#include "../ec-cpp/ec-cpp.hpp"
#include "../../ec-cpp/f2e16.hpp"
#include "../../ec-cpp/table_f2e16.hpp"

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

auto createTestChunks(ChunksList &out, uint64_t validators = n_validators) {
  DataBlock data{.array = (uint8_t *)test_data.data(),
                 .length = test_data.length()};
  return ECCR_obtain_chunks(validators, &data, &out);
}

TEST(erasure_coding, Cpp_AFFT_tables) {
    uint16_t src_0[65535];
    ECCR_AFFT_Table(&src_0);

    ec_cpp::PolyEncoder_f2e16 init{};
    auto src_1 = ec_cpp::PolyEncoder_f2e16::AdditiveFFT::initalize(init.kTables);

    ASSERT_EQ(sizeof(src_0) / sizeof(src_0[0]), sizeof(src_1.skews) / sizeof(src_1.skews[0]));
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
    ec_cpp::PolyEncoder_f2e16 poly;
    auto &[log_table, exp_table, walsh_table] = poly.kTables;

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
  for (size_t i = 0ull; i < 1000ull; ++i)
    ASSERT_EQ(Prototype_log2(i), ec_cpp::math::log2(i));
}

TEST(erasure_coding, Cpp_CreateChunks) {
    ec_cpp::create();
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

/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>

#include "../ec-cpp/ec-cpp.hpp"
#include "../../ec-cpp/f2e16.hpp"

extern "C" {
#include <erasure_coding/erasure_coding.h>
}

static constexpr std::string_view test_data =
    "This is a test string. The purpose of it is not allow the evil forces to "
    "conquer the world!";
static constexpr uint64_t n_validators = 6ull;

auto createTestChunks(ChunksList &out, uint64_t validators = n_validators) {
  DataBlock data{.array = (uint8_t *)test_data.data(),
                 .length = test_data.length()};
  return ECCR_obtain_chunks(validators, &data, &out);
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
    int p = 0; ++p;
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

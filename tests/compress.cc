#include "compress.h"
#include <gtest/gtest.h>
#include <iomanip>

TEST(CompressionTest, CompressionTest1) {
  Compressor compressor("nekochan");
  std::vector<uint8_t> output = compressor.Compress();
  compressor.UpdateBuffer(std::move(output));
  output = compressor.Decompress();
  std::string result(output.begin(), output.end());
  ASSERT_EQ(result,"nekochan");
  //   for (int i = 0; i < output.size(); i++) {
  //     std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0')
  //               << static_cast<int>(output[i]);

  //     if (i != output.size() - 1)
  //       std::cout << ", ";
  //   }
}
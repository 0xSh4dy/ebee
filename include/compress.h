#ifndef COMPRESS_H
#define COMPRESS_H

#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>
#include <vector>
#include <zlib.h>

class Compressor {
public:
  Compressor(const std::string &input) {
    input_.assign(input.begin(), input.end());
  }

  Compressor(const uint8_t *data, size_t len) {
    input_.assign(data, data + len);
  }

  Compressor(const std::vector<uint8_t>&data){
    input_ = data;
  }

  std::vector<uint8_t>Compress(){
    std::vector<uint8_t>output;
    if(!compress_gzip(input_.data(),input_.size(),output)){
        throw std::runtime_error("failure");
    }
    return output;
  }

  void UpdateBuffer(const std::vector<uint8_t> &&buf){
    input_ = std::move(buf);
  }

  std::vector<uint8_t>Decompress(){
    std::vector<uint8_t>output;
    if(!decompress_gzip(input_.data(),input_.size(),output)){
        throw std::runtime_error("failure");
    }
    return output;
  }

private:
  std::vector<uint8_t> input_;
  bool decompress_gzip(const uint8_t *input, size_t input_size,
                       std::vector<uint8_t> &output) {
    z_stream strm = {};
    strm.next_in = const_cast<Bytef *>(reinterpret_cast<const Bytef *>(input));
    strm.avail_in = input_size;

    if (inflateInit2(&strm, 15 + 16) != Z_OK) {
      std::cerr << "failed" << std::endl;
      return false;
    }

    const size_t chunk_size = 4096;
    uint8_t temp[chunk_size];

    int ret;
    do {
      strm.avail_out = chunk_size;
      strm.next_out = temp;

      ret = inflate(&strm, Z_NO_FLUSH);
      if (ret == Z_STREAM_ERROR || ret == Z_DATA_ERROR || ret == Z_MEM_ERROR) {
        std::cerr << "error" << ret << std::endl;
        inflateEnd(&strm);
        return false;
      }

      size_t bytes_decompressed = chunk_size - strm.avail_out;
      output.insert(output.end(), temp, temp + bytes_decompressed);
    } while (ret != Z_STREAM_END);

    inflateEnd(&strm);
    return true;
  }

  bool compress_gzip(const uint8_t *input, size_t input_size,
                     std::vector<uint8_t> &output,
                     int compression_level = Z_BEST_COMPRESSION) {
    z_stream strm = {};
    strm.next_in = const_cast<Bytef *>(input);
    strm.avail_in = input_size;

    if (deflateInit2(&strm, compression_level, Z_DEFLATED, 15 + 16, 8,
                     Z_DEFAULT_STRATEGY) != Z_OK) {
      std::cerr << "failed\n";
      return false;
    }

    const size_t chunk_size = 4096;
    uint8_t temp[chunk_size];

    int ret;
    do {
      strm.avail_out = chunk_size;
      strm.next_out = temp;

      ret = deflate(&strm, strm.avail_in ? Z_NO_FLUSH : Z_FINISH);
      if (ret == Z_STREAM_ERROR) {
        std::cerr << "failed\n";
        deflateEnd(&strm);
        return false;
      }

      size_t bytes_compressed = chunk_size - strm.avail_out;
      output.insert(output.end(), temp, temp + bytes_compressed);
    } while (ret != Z_STREAM_END);

    deflateEnd(&strm);
    return true;
  }
};

#endif
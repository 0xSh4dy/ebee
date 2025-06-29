#include <iostream>
#include <vector>

#include <immintrin.h>

int recursive_sum(const int *ptr, size_t n) {
  if (n == 0)
    return 0;

  alignas(32) int temp[8] = {0};

  if (n >= 8) {
    __m256i chunk = _mm256_loadu_si256((__m256i const *)(ptr));
    _mm256_store_si256((__m256i *)temp, chunk);
    int subtotal = 0;
    for (int i = 0; i < 8; ++i)
      subtotal += temp[i];
    return subtotal + recursive_sum(ptr + 8, n - 8);
  } else {
    for (size_t i = 0; i < n; ++i)
      temp[i] = ptr[i];
    __m256i tail = _mm256_loadu_si256((__m256i const *)temp);
    _mm256_store_si256((__m256i *)temp, tail);
    int subtotal = 0;
    for (int i = 0; i < 8; ++i)
      subtotal += temp[i];
    return subtotal;
  }
}

int add(const std::vector<int> &nums) {
  return recursive_sum(nums.data(), nums.size());
}

int main() {
  int x = add({1, 2, 3, 4, 5, 6, 7, 8, 9, 10});
  std::cout << "Value " << x;
  return 0;
}
#include "challenge.h"
#include "compress.h"
#include "executor.h"

#include <format>
#include <immintrin.h>
#include <iomanip>
#include <stdlib.h>

#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/TargetSelect.h>

#define EX executor.Execute
#define EXFMT(fmt, ...) EX(std::format(fmt, __VA_ARGS__))

bool CheckFirst6ModifiedChars(const std::vector<uint8_t> &res) {
  if (res.size() < 6)
    return false;
  alignas(16) uint8_t actual[16] = {0};

  for (int i = 0; i < 6; ++i) {
    actual[i] = res[i];
  }

  alignas(16) const uint8_t expected[16] = {79, 79, 151, 51, 222, 253};

  __m128i a = _mm_load_si128(reinterpret_cast<const __m128i *>(actual));
  __m128i b = _mm_load_si128(reinterpret_cast<const __m128i *>(expected));

  __m128i cmp = _mm_cmpeq_epi8(a, b);
  return (_mm_movemask_epi8(cmp) & 0b111111) == 0b111111;
}

static std::vector<uint8_t> GetRc4Module() {
  std::vector<uint8_t> result;
  result.reserve(kRc4Module.size());

  for (int i = 0; i < kRc4Module.size(); i++) {
    result.push_back(kRc4Module[i] ^ 1337);
  }
  return result;
}

static std::vector<uint8_t> GetKeyModule() {
  std::vector<uint8_t> result;
  result.reserve(kKeyModule.size());

  for (int i = 0; i < kKeyModule.size(); i++) {
    result.push_back(kKeyModule[i] ^ 21);
  }
  return result;
}

static char *GetKey1(llvm::orc::LLJIT *jit) {
  auto sym = jit->lookup("t31").get();
  if (!sym) {
    throw std::runtime_error("nah");
  }
  using t31_type = char *(*)();
  auto *fn = reinterpret_cast<t31_type>(sym.getValue());
  return fn();
}

static char *GetKey2(llvm::orc::LLJIT *jit) {
  auto sym = jit->lookup("t32").get();
  if (!sym) {
    throw std::runtime_error("nah");
  }
  using t32_type = char *(*)();
  auto *fn = reinterpret_cast<t32_type>(sym.getValue());
  return fn();
}

static std::vector<uint8_t> ComputeRc4(llvm::orc::LLJIT *jit, uint8_t *buffer,
                                       char *key, int size) {
  std::vector<uint8_t> result;
  result.resize(size);
  auto sym = jit->lookup("t2").get();
  if (!sym) {
    throw std::runtime_error("nah");
  }
  using t2_type = int (*)(char *, unsigned char *, unsigned char *, int);
  auto *fn = reinterpret_cast<t2_type>(sym.getValue());
  fn(key, buffer, result.data(), size);
  return result;
}

static std::vector<uint8_t> GetCrc32Module(llvm::orc::LLJIT *jit) {
  std::vector<uint8_t> result;
  result = ComputeRc4(jit, const_cast<uint8_t *>(kCrc32Module.data()),
                      GetKey1(jit), kCrc32Module.size());
  return result;
}

static void AddModule(llvm::LLVMContext &ctx, llvm::orc::LLJIT *jit,
                      const std::vector<uint8_t> &result,
                      const std::string &name) {
  std::unique_ptr<llvm::MemoryBuffer> buffer =
      llvm::MemoryBuffer::getMemBufferCopy(
          llvm::StringRef(reinterpret_cast<const char *>(result.data()),
                          result.size()),
          name);
  auto module_or_err = llvm::parseBitcodeFile(buffer->getMemBufferRef(), ctx);
  if (!module_or_err) {
    throw std::runtime_error(
        "nah");
  }

  if (auto err = jit->addIRModule(
          llvm::orc::ThreadSafeModule(std::move(module_or_err.get()),
                                      std::make_unique<llvm::LLVMContext>()))) {
    llvm::logAllUnhandledErrors(std::move(err), llvm::errs(),
                                "nah: ");
    throw std::runtime_error(
        "nah");
  }
}

static unsigned int ComputeCrc32(llvm::orc::LLJIT *jit, const uint8_t *buffer,
                                 int size) {
  auto sym = jit->lookup("t1").get();
  if (!sym) {
    throw std::runtime_error("nah");
  }
  using t1_type = int (*)(const uint8_t *, unsigned int);
  auto *fn = reinterpret_cast<t1_type>(sym.getValue());
  return fn(buffer, size);
}

static void SetupKeyLoader(llvm::LLVMContext &ctx, llvm::orc::LLJIT *jit) {
  Compressor compressor(GetKeyModule());
  std::vector<uint8_t> result = compressor.Decompress();
  AddModule(ctx, jit, result, "t3m");
}

static void SetupRc4Loader(llvm::LLVMContext &ctx, llvm::orc::LLJIT *jit) {
  Compressor compressor(GetRc4Module());
  std::vector<uint8_t> result = compressor.Decompress();
  AddModule(ctx, jit, result, "t2m");
}

static void SetupCrc32Loader(llvm::LLVMContext &ctx, llvm::orc::LLJIT *jit) {
  Compressor compressor(GetCrc32Module(jit));
  std::vector<uint8_t> result = compressor.Decompress();
  AddModule(ctx, jit, result, "t1m");
}

static inline llvm::LLVMContext CreateContext() { return llvm::LLVMContext(); }

void StartChallenge(uint8_t *payload, int size) {
  llvm::LLVMContext ctx = CreateContext();
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmParser();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::orc::LLJITBuilder jit_builder = llvm::orc::LLJITBuilder();
  bool error = false;

  auto jit = jit_builder.create();
  if (!jit) {
    throw std::runtime_error("failed to start the challenge");
  }

  SetupKeyLoader(ctx, jit->get());
  SetupRc4Loader(ctx, jit->get());
  SetupCrc32Loader(ctx, jit->get());

  const uint8_t *input = reinterpret_cast<const uint8_t *>("nekochan");

  uint8_t *temp = (uint8_t *)calloc(1, size);
  for (int i = 0; i < 4; i++) {
    temp[i] = payload[i] ^ payload[i + 4];
  }

  if (ComputeCrc32(jit->get(), temp, 4) != 0xfeb9a9be) {
    error = true;
  }

  memcpy(temp, payload, 6);
  std::vector<uint8_t> res =
      ComputeRc4(jit->get(), temp, GetKey2(jit->get()), 6);

  if (!CheckFirst6ModifiedChars(res)) {
    error = true;
  }

  if(error){
    throw std::runtime_error("nahh");
  }
  memcpy(temp, payload + 11, 9);
  ebee::Executor executor;

  EXFMT("{} {}", EPUSH, 150);
  EXFMT("{} {}", EPUSH, 96);
  EX(EAND);
  EXFMT("{} {}", EPUSH, 150);
  EXFMT("{} {}", EPUSH, 96);
  EX(EOR);
  EX(EADD);

  EXFMT("{} {}", EPUSH, 5);
  EXFMT("{} {}", EPUSH, 6);
  EX(EAND);
  EXFMT("{} {}", EPUSH, 5);
  EXFMT("{} {}", EPUSH, 6);
  EX(EOR);
  EX(EADD);
  EXFMT("{} {}", EPUSH, temp[0]);
  EX(EADD);

  EXFMT("{} {}", EPUSH, temp[1]);
  EXFMT("{} {}", EPUSH, -10);
  EX(EXOR);
  EXFMT("{} {}", EPUSH, temp[1]);
  EXFMT("{} {}", EPUSH, -10);
  EX(EAND);
  EXFMT("{} {}", EPUSH, 2);
  EX(EMUL);
  EX(EADD);
  EXFMT("{} {}", EPUSH, 72);
  EX(EADD);
  EX(ECMP);
  EX(EEXIT);

  EXFMT("{} {}", EPUSH, temp[1]);
  EXFMT("{} {}", EPUSH, -2);
  EX(EXOR);
  EXFMT("{} {}", EPUSH, temp[1]);
  EXFMT("{} {}", EPUSH, -2);
  EX(EAND);
  EXFMT("{} {}", EPUSH, 2);
  EX(EMUL);
  EX(EADD);
  EXFMT("{} {}", EPUSH, 200);
  EX(EADD);
  EX(ECMP);
  EX(EEXIT);

  EXFMT("{} {}", EPUSH, 11);
  EXFMT("{} {}", EPUSH, temp[2]);
  EXFMT("{} {}", EPUSH, 11);
  EX(EOR);
  EXFMT("{} {}", EPUSH, temp[2]);
  EXFMT("{} {}", EPUSH, 11);
  EX(EAND);
  EX(EADD);
  EX(ESUB);

  EXFMT("{} {}", EPUSH, temp[3]);
  EXFMT("{} {}", EPUSH, -50);
  EX(EXOR);
  EXFMT("{} {}", EPUSH, temp[3]);
  EXFMT("{} {}", EPUSH, -50);
  EX(EAND);
  EXFMT("{} {}", EPUSH, 2);
  EX(EMUL);
  EX(EADD);
  EXFMT("{} {}", EPUSH, 10);
  EXFMT("{} {}", EPUSH, 5);
  EX(EMUL);
  EX(EADD);

  EXFMT("{} {}", EPUSH, 20);
  EXFMT("{} {}", EPUSH, temp[4]);
  EX(ESUB);
  EXFMT("{} {}", EPUSH, 5);
  EXFMT("{} {}", EPUSH, 4);
  EX(EMUL);
  EX(EADD);

  EXFMT("{} {}", EPUSH, 40);
  EXFMT("{} {}", EPUSH, 31);
  EXFMT("{} {}", EPUSH, 29);
  EX(EADD);
  EXFMT("{} {}", EPUSH, temp[5]);
  EXFMT("{} {}", EPUSH, 100);
  EX(EADD);
  EX(ESUB);
  EX(ESUB);

  EXFMT("{} {}", EPUSH, 116);
  EX(ECMP);
  EX(EEXIT);

  EXFMT("{} {}", EPUSH, -13);
  EXFMT("{} {}", EPUSH, 15);
  EXFMT("{} {}", EPUSH, 51);
  EX(EXOR);
  EX(ESUB);
  EX(ECMP);
  EX(EEXIT);

  EXFMT("{} {}", EPUSH, 10);
  EXFMT("{} {}", EPUSH, 500);
  EX(EDIV);
  EXFMT("{} {}", EPUSH, 2);
  EX(EMUL);
  EXFMT("{} {}", EPUSH, 9);
  EX(EADD);

  EX(ECMP);
  EX(EEXIT);

  EXFMT("{} {}", EPUSH, 77);
  EX(ECMP);
  EX(EEXIT);

  EXFMT("{} {}", EPUSH, temp[6]);
  EXFMT("{} {}", EPUSH, temp[7]);
  EXFMT("{} {}", EPUSH, 2);
  EXFMT("{} {}", EPUSH, 4);
  EX(EDIV);
  EX(EMUL);
  EX(EADD);
  EXFMT("{} {}", EPUSH, 186);
  EX(ECMP);
  EX(EEXIT);

  EXFMT("{} {}", EPUSH, temp[6]);
  EXFMT("{} {}", EPUSH, temp[6]);
  EX(EOR);
  EXFMT("{} {}", EPUSH, temp[6]);
  EXFMT("{} {}", EPUSH, temp[6]);
  EX(EAND);
  EX(EADD);
  EXFMT("{} {}", EPUSH, temp[8]);
  EX(EADD);
  EXFMT("{} {}", EPUSH, 300);
  EXFMT("{} {}", EPUSH, -32);
  EX(EXOR);
  EXFMT("{} {}", EPUSH, 300);
  EXFMT("{} {}", EPUSH, -32);
  EX(EAND);
  EXFMT("{} {}", EPUSH, 2);
  EX(EMUL);
  EX(EADD);
  EX(ECMP);
  EX(EEXIT);

  EXFMT("{} {}", EPUSH, temp[6]);
  EXFMT("{} {}", EPUSH, 5);
  EX(EMUL);
  EXFMT("{} {}", EPUSH, temp[7]);
  EXFMT("{} {}", EPUSH, 20);
  EX(EMUL);
  EX(EADD);
  EXFMT("{} {}", EPUSH, temp[8]);
  EXFMT("{} {}", EPUSH, 2);
  EX(EMUL);
  EX(EADD);
  EXFMT("{} {}", EPUSH, 100);
  EXFMT("{} {}", EPUSH, 1000);
  EX(EOR);
  EXFMT("{} {}", EPUSH, 100);
  EXFMT("{} {}", EPUSH, 1000);
  EX(EAND);
  EXFMT("{} {}", EPUSH, -1);
  EX(EMUL);
  EX(EADD);
  EXFMT("{} {}", EPUSH, 692);
  EX(EADD);
  EXFMT("{} {}", EPUSH, 2);
  EXFMT("{} {}", EPUSH, 80);
  EX(EDIV);
  EX(EADD);
  EX(ECMP);
  EX(EEXIT);

  if (executor.HasErrors()) {
    throw std::runtime_error("nahh");
  }
  free(temp);
  std::cout<<"DEAD{"<<std::string((char*)payload)<<"}\n";
}

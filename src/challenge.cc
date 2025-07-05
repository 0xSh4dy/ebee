#include "challenge.h"
#include "compress.h"
#include "executor.h"

#include <immintrin.h>
#include <iomanip>
#include <format>
#include <stdlib.h>

#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/TargetSelect.h>

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
    throw std::runtime_error("cannot run the challenge");
  }
  using t31_type = char *(*)();
  auto *fn = reinterpret_cast<t31_type>(sym.getValue());
  return fn();
}

static char *GetKey2(llvm::orc::LLJIT *jit) {
  auto sym = jit->lookup("t32").get();
  if (!sym) {
    throw std::runtime_error("cannot run the challenge");
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
    throw std::runtime_error("cannot run the challenge");
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
        "cannot run the challenge, failed to parse the bytecode");
  }

  if (auto err = jit->addIRModule(
          llvm::orc::ThreadSafeModule(std::move(module_or_err.get()),
                                      std::make_unique<llvm::LLVMContext>()))) {
    llvm::logAllUnhandledErrors(std::move(err), llvm::errs(),
                                "addIRModule failed: ");
    throw std::runtime_error(
        "cannot run the challenge, failed to add the IR module");
  }
}

static unsigned int ComputeCrc32(llvm::orc::LLJIT *jit, const uint8_t *buffer,
                                 int size) {
  auto sym = jit->lookup("t1").get();
  if (!sym) {
    throw std::runtime_error("cannot run the challenge");
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


static inline llvm::LLVMContext CreateContext(){
    return llvm::LLVMContext();
}

void StartChallenge(uint8_t *payload, int size) {
  llvm::LLVMContext ctx = CreateContext();
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmParser();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::orc::LLJITBuilder jit_builder = llvm::orc::LLJITBuilder();

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
    throw std::runtime_error("nah");
  }

  memcpy(temp, payload, 6);
  std::vector<uint8_t> res =
      ComputeRc4(jit->get(), temp, GetKey2(jit->get()), 6);

  if (!CheckFirst6ModifiedChars(res)) {
    throw std::runtime_error("nah");
  }

  memcpy(temp,payload+11,9);
  ebee::Executor executor;

  executor.Execute("epush 246");
  executor.Execute(std::format("eadd {} 11",temp[0]));
  executor.Execute(std::format("eadd {} 62",temp[1]));
  executor.Execute("ecmp");
  executor.Execute("eexit");
  executor.Execute(std::format("eadd 198 {}",temp[1]));
  executor.Execute("ecmp");
  executor.Execute("eexit");
  executor.Execute(std::format("epush {}",temp[2]));
  executor.Execute(std::format("epush {}",temp[3]));

  if(executor.HasErrors()){
    std::cout<<"Nah\n";
  }
  free(temp);
  // std::cout << std::hex << ComputeCrc32(jit->get(), input, 8) << std::endl;

  // std::vector<uint8_t> res =
  //     ComputeRc4(jit->get(), const_cast<uint8_t *>(input), "meow", 8);
  //   for (auto it : res) {
  //     std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)it
  //               << " ";
  //   }
}

//   std::string res(result.begin(), result.end());
//   std::unique_ptr<llvm::MemoryBuffer> mem_buffer =
//       llvm::MemoryBuffer::getMemBuffer(res, "");
//   std::unique_ptr<llvm::Module> module =
//       llvm::parseIR(mem_buffer->getMemBufferRef(), error, ctx);

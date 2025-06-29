#include "executor.h"

#include <gtest/gtest.h>

using namespace ebee;

TEST(TestExecutor, InstructionBuilding) {
  Executor executor;
  std::vector<Instruction *> instructions = executor.GetInstructions();
  ASSERT_EQ(instructions.size(), 11);
}

TEST(TestExecutor, JitExecutor) {
  std::vector<std::string> instr_strs = {"eadd 1 2", "esub 10 5", "emul 5 2",
                                         "ecmp","eexit"};
  Executor executor;

  for (auto &instr : instr_strs) {
    executor.Execute(instr);
  }
}
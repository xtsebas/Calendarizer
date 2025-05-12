#include "file_loader.h"
#include <gtest/gtest.h>

TEST(FileLoaderTest, LoadValidFile) {
  auto procs = FileLoader::loadProcesses("tests/data/processes_valid.txt");
  EXPECT_EQ(procs.size(), 3);
  EXPECT_EQ(procs[0].pid, "P1");
}

TEST(FileLoaderTest, LoadInvalidFile) {
  EXPECT_THROW(
    FileLoader::loadProcesses("tests/data/processes_invalid.txt"),
    std::runtime_error);
}

/// \file error.test.cc
/// \brief Error handling API tests.
/// \author Amlal El Mahrouss (amlal at nekernel dot org)

#include <SystemKit/System.h>
#include <public/frameworks/KernelTest/headers/TestCase.h>

/// \note ErrGetLastError tests
KT_DECL_TEST(ErrGetLastErrorInitial, []() -> bool {
  SInt32 error = ErrGetLastError();
  return error >= 0;
});

KT_DECL_TEST(ErrGetLastErrorAfterSuccess, []() -> bool {
  auto heap = MmCreateHeap(1024, 0);
  if (!heap) return NO;

  SInt32 error = ErrGetLastError();
  MmDestroyHeap(heap);

  return error == 0;
});

KT_DECL_TEST(ErrGetLastErrorAfterFailure, []() -> bool {
  auto heap = MmCreateHeap(0, 0);
  MUST_PASS(heap);
  if (!heap) return NO;

  SInt32 error = ErrGetLastError();

  return error != 0;
});

KT_DECL_TEST(ErrGetLastErrorAfterInvalidFile, []() -> bool {
  auto file = IoOpenFile("/invalid/path/that/does/not/exist", nullptr);

  SInt32 error = ErrGetLastError();

  if (file) IoCloseFile(file);

  return error != 0;
});

KT_DECL_TEST(ErrGetLastErrorAfterNullOp, []() -> bool {
  auto ptr = MmCopyMemory(nullptr, nullptr, 10);

  MUST_PASS(ptr);
  if (!ptr) return NO;

  SInt32 error = ErrGetLastError();

  return error != 0;
});

KT_DECL_TEST(ErrGetLastErrorMultipleCalls, []() -> bool {
  SInt32 error1 = ErrGetLastError();
  SInt32 error2 = ErrGetLastError();

  return error1 == error2;
});

/// \brief Run error tests.
IMPORT_C SInt32 KT_TEST_MAIN() {
  KT_RUN_TEST(ErrGetLastErrorInitial);
  KT_RUN_TEST(ErrGetLastErrorAfterSuccess);
  KT_RUN_TEST(ErrGetLastErrorAfterFailure);
  KT_RUN_TEST(ErrGetLastErrorAfterInvalidFile);
  KT_RUN_TEST(ErrGetLastErrorAfterNullOp);
  KT_RUN_TEST(ErrGetLastErrorMultipleCalls);

  return KT_TEST_SUCCESS;
}

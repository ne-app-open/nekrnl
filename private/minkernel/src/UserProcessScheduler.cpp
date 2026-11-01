// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#include <ArchKit/ArchKit.h>
#include <KernelKit/HardwareThreadScheduler.h>
#include <KernelKit/HeapMgr.h>
#include <KernelKit/IPEFDylibObject.h>
#include <KernelKit/KPC.h>
#include <KernelKit/ProcessScheduler.h>
#include <NeKit/KString.h>
#include <NeKit/Utils.h>
#include <SignalKit/Signal.h>

///! BUG COUNT: 0

namespace Ne::Kernel {

UserProcess::UserProcess()  = default;
UserProcess::~UserProcess() = default;

/***********************************************************************************/
/// @brief Gets the last exit code.
/// @note Not thread-safe.
/// @return Int32 the last exit code.
/***********************************************************************************/

/***********************************************************************************/
/// @brief Crashes the current process.
/***********************************************************************************/

Void UserProcess::Crash() {
  if (this->Status != ProcessStatusKind::kRunning) return;

  this->Status = ProcessStatusKind::kKilled;

  (Void)(kout << this->Name << ": crashed, error id: " << number(-kErrorProcessFault) << kendl);
}

/***********************************************************************************/
/// @brief boolean operator, check status.
/***********************************************************************************/

UserProcess::operator bool() {
  return this->Status == ProcessStatusKind::kRunning;
}

/***********************************************************************************/
/// @brief Gets the local last exit code.
/// @note Not thread-safe.
/// @return Int32 the last exit code.
/***********************************************************************************/

KPCError& UserProcess::GetExitCode() {
  return this->LastExitCode;
}

/***********************************************************************************/
/// @brief Error code variable getter.
/***********************************************************************************/

KPCError& UserProcess::GetLocalCode() {
  return this->LocalCode;
}

/***********************************************************************************/
/// @brief Wakes process header.
/// @param should_wakeup if the program shall wakeup or not.
/***********************************************************************************/

Void UserProcess::Wake(Bool should_wakeup) {
  this->Status = should_wakeup ? ProcessStatusKind::kRunning : ProcessStatusKind::kFrozen;
}

/***********************************************************************************/
/** @brief Allocate pointer to heap tree. */
/** @param tree The tree to calibrate */
/***********************************************************************************/

template <typename T>
STATIC T* sched_try_go_upper_ptr_tree(T* tree) {
  if (!tree) {
    return nullptr;
  }

  if (tree) {
    if (tree->Parent) tree = tree->Parent;

    auto tree_tmp = tree->Next;

    if (!tree_tmp) {
      return tree;
    }

    return tree_tmp;
  }

  return tree;
}

/***********************************************************************************/
/** @brief Allocate pointer to heap/file tree. */
/***********************************************************************************/

ErrorOr<VoidPtr> UserProcess::New(SizeT sz, SizeT pad_amount) {
  if (this->UsedMemory > kCPSMaxMemoryLimit) return ErrorOr<VoidPtr>(-kErrorHeapOutOfMemory);

#ifdef __NE_VIRTUAL_MEMORY_SUPPORT__
  auto vm_register = kKernelVM;

  hal_write_cr3(this->VMRegister);

  auto ptr = mm_alloc_ptr(sz, Yes, Yes, pad_amount);

  hal_write_cr3(vm_register);
#else
  auto ptr = mm_alloc_ptr(sz, Yes, Yes, pad_amount);
#endif

  if (!this->HeapTree) {
    this->HeapTree = new ProcessHeapTree<VoidPtr>();

    if (!this->HeapTree) {
      this->Crash();
      return ErrorOr<VoidPtr>(-kErrorHeapOutOfMemory);
    }

    this->HeapTree->EntryPad  = pad_amount;
    this->HeapTree->EntrySize = sz;

    this->HeapTree->Entry = ptr;

    this->HeapTree->Color = TreeKind::kBlackTreeKind;

    this->HeapTree->Prev   = nullptr;
    this->HeapTree->Next   = nullptr;
    this->HeapTree->Parent = nullptr;
    this->HeapTree->Child  = nullptr;
  } else {
    ProcessHeapTree<VoidPtr>* entry      = this->HeapTree;
    ProcessHeapTree<VoidPtr>* prev_entry = entry;

    BOOL is_parent = NO;

    while (entry) {
      if (entry->EntrySize < 1) break;

      prev_entry = entry;

      if (entry->Child && entry->Child->EntrySize > 0 && entry->Child->EntrySize == sz) {
        entry     = entry->Child;
        is_parent = YES;
      } else if (entry->Next && entry->Next->EntrySize > 0 && entry->Next->EntrySize == sz) {
        is_parent = NO;
        entry     = entry->Next;
      } else {
        entry = sched_try_go_upper_ptr_tree(entry);
        if (entry && entry->Color == TreeKind::kBlackTreeKind) break;
      }
    }

    auto new_entry = new ProcessHeapTree<VoidPtr>();

    if (!new_entry) {
      this->Crash();
      return ErrorOr<VoidPtr>(-kErrorHeapOutOfMemory);
    }

    new_entry->Entry     = ptr;
    new_entry->EntrySize = sz;
    new_entry->EntryPad  = pad_amount;
    new_entry->Parent    = entry;
    new_entry->Child     = nullptr;
    new_entry->Next      = nullptr;
    new_entry->Prev      = nullptr;

    new_entry->Color  = TreeKind::kBlackTreeKind;
    prev_entry->Color = TreeKind::kRedTreeKind;

    if (is_parent) {
      prev_entry->Child = new_entry;
      new_entry->Parent = prev_entry;
    } else {
      prev_entry->Next = new_entry;
      new_entry->Prev  = prev_entry;
    }
  }

  this->UsedMemory += sz;

  return ErrorOr<VoidPtr>(ptr);
}

/***********************************************************************************/
/// @brief Gets the name of the current process.
/***********************************************************************************/

const Char* UserProcess::GetName() {
  return this->Name;
}

/***********************************************************************************/
/// @brief Gets the owner of the process.
/***********************************************************************************/

const Ref<User*> UserProcess::GetOwner() {
  return {this->Owner};
}

/// @brief Parent team getter.
UserProcessTeam* UserProcess::GetParentTeam() {
  return this->Parent;
}

/***********************************************************************************/
/** @brief Free heap tree. */
/***********************************************************************************/

template <typename T>
STATIC Void sched_free_ptr_tree(T* tree) {
  // Deleting memory lists. Make sure to free all of them.
  while (tree) {
    if (tree->Entry) {
      MUST_PASS(mm_free_ptr(tree->Entry) == kErrorSuccess);
    }

    auto next = tree->Next;

    if (next && next->Child) sched_free_ptr_tree(next->Child);

    tree->Child = nullptr;

    mm_free_ptr(tree);

    tree = next;
  }
}

/***********************************************************************************/
/**
@brief Exit process method.
@param exit_code The process's exit code.
*/
/***********************************************************************************/

Void UserProcess::Exit(const Int32& exit_code) {
  this->Status       = exit_code > 0 ? ProcessStatusKind::kKilled : ProcessStatusKind::kFrozen;
  this->LastExitCode = exit_code;

#ifdef __NE_VIRTUAL_MEMORY_SUPPORT__
  auto pd = kKernelVM;
  hal_write_cr3(this->VMRegister);
#endif

  sched_free_ptr_tree(this->HeapTree);
  this->HeapTree = nullptr;

  sched_free_ptr_tree(this->FileTree);
  this->FileTree = nullptr;

#ifdef __NE_VIRTUAL_MEMORY_SUPPORT__
  hal_write_cr3(pd);
#endif

#ifdef __NE_VIRTUAL_MEMORY_SUPPORT__
  //! Free the memory's page directory.
  if (this->VMRegister) HAL::mm_free_bitmap(this->VMRegister);
#endif

  //! Delete image if not done already.
  if (this->Image.fCode && mm_is_valid_ptr(this->Image.fCode)) mm_free_ptr(this->Image.fCode);
  //! Delete blob too.
  if (this->Image.fBlob && mm_is_valid_ptr(this->Image.fBlob)) mm_free_ptr(this->Image.fBlob);

  //! Delete stack frame.
  if (this->StackFrame && mm_is_valid_ptr(this->StackFrame))
    mm_free_ptr((VoidPtr) this->StackFrame);

  //! Avoid use after free.
  this->Image.fBlob = nullptr;
  this->Image.fCode = nullptr;
  this->StackFrame  = nullptr;

  if (this->Kind == ExecutableKind::kExecutableDylibKind) {
    Bool success = false;

    rtl_fini_dylib_pef(*this, reinterpret_cast<IPEFDylibObject*>(this->DylibDelegate), &success);

    if (!success) {
      ke_stop(RUNTIME_CHECK_PROCESS);
    }

    this->DylibDelegate = nullptr;
  }

  this->ProcessId = 0UL;
  this->Status    = ProcessStatusKind::kFinished;

  --this->Parent->mProcessCur;
}

/***********************************************************************************/
/// @brief Initializer dylib of User Process.
/***********************************************************************************/

Bool UserProcess::InitDylib() {
  // React according to the process's kind.
  switch (this->Kind) {
    case ExecutableKind::kExecutableDylibKind: {
      this->DylibDelegate = rtl_init_dylib_pef(*this);

      if (!this->DylibDelegate) {
        this->Crash();
        return NO;
      }

      return YES;
    }
    case ExecutableKind::kExecutableKind: {
      return NO;
    }
    default: {
      break;
    }
  }

  (Void)(kout << "Unknown process kind: " << hex_number(static_cast<Int32>(this->Kind)) << kendl);
  this->Crash();

  return NO;
}

/***********************************************************************************/
/// @brief Add process to team.
/// @param process the process *Ref* class.
/// @return the process index inside the team.
/***********************************************************************************/

ProcessID UserProcessScheduler::Spawn(const Char* name, VoidPtr code, VoidPtr image,
                                      SizeT image_sz) {
  if (!name || !code) {
    return -kErrorProcessFault;
  }

  if (*name == 0) {
    return -kErrorProcessFault;
  }

  ProcessID pid = this->mTeam.mProcessCur;

  if (pid > kCPSProcessLimitPerTeam) {
    return -kErrorProcessFault;
  }

  ++this->mTeam.mProcessCur;

  UserProcess& process = this->mTeam.mProcessList[pid];

  process.Image.fCode   = code;
  process.Image.fBlob   = image;
  process.Image.fBlobSz = image_sz;

  SizeT len = rt_string_len(name);

  if (len > kCPSNameLen) {
    return -kErrorProcessFault;
  }

  rt_copy_memory_safe(reinterpret_cast<VoidPtr>(const_cast<Char*>(name)), process.Name, len,
                      kCPSNameLen);

#ifdef __NE_VIRTUAL_MEMORY_SUPPORT__
  process.VMRegister = kKernelVM;
#else
  process.VMRegister = 0UL;
#endif  // ifdef __NE_VIRTUAL_MEMORY_SUPPORT__

  process.StackFrame = new HAL::StackFrame();

  if (!process.StackFrame) {
    process.Crash();
    return -kErrorProcessFault;
  }

  rt_set_memory(process.StackFrame, 0, sizeof(HAL::StackFrame));

  process.StackFrame->IP = reinterpret_cast<UIntPtr>(code);
  process.StackFrame->SP = reinterpret_cast<UIntPtr>(&process.StackReserve[0] + process.StackSize);

#if defined(__NE_VIRTUAL_MEMORY_SUPPORT__)
  HAL::mm_map_page((VoidPtr) process.StackFrame->IP,
                   (VoidPtr) HAL::mm_get_page_addr((VoidPtr) process.StackFrame->IP),
                   HAL::kMMFlagsUser | HAL::kMMFlagsPresent);
  HAL::mm_map_page((VoidPtr) process.StackFrame->SP,
                   (VoidPtr) HAL::mm_get_page_addr((VoidPtr) process.StackFrame->SP),
                   HAL::kMMFlagsUser | HAL::kMMFlagsPresent);
#endif  // ifdef __NE_VIRTUAL_MEMORY_SUPPORT__

  process.StackSize = kCPSMaxStackSz;

  rt_set_memory(process.StackReserve, 0, process.StackSize);

  process.Parent = &mTeam;

  process.ProcessId = pid;
  process.Status    = ProcessStatusKind::kRunning;
  process.PTime     = 0;

  /// @note the team is copied in from a global whose constructor never ran, so the
  /// member initialisers are gone. Spawn owns the scheduling state.
  process.Affinity = AffinityKind::kStandard;
  process.RTime    = 0;
  process.UTime    = 0;
  process.STime    = 0;

  if (!process.FileTree) {
    process.FileTree = new ProcessFileTree<VoidPtr>();

    if (!process.FileTree) {
      process.Crash();
      return -kErrorHeapOutOfMemory;
    }

    /// @todo File Tree allocation and dispose methods (amlal)
  }

  (Void)(kout << "ProcessCur: " << number(this->mTeam.mProcessCur) << kendl);
  (Void)(kout << "ProcessID: " << number(pid) << kendl);
  (Void)(kout << "ProcesName: " << process.Name << kendl);

  return pid;
}

/***********************************************************************************/
/// @brief Retrieves the singleton of the process scheduler.
/***********************************************************************************/

UserProcessScheduler& UserProcessScheduler::The() {
  STATIC UserProcessScheduler kUserScheduler;
  return kUserScheduler;
}

/***********************************************************************************/

/// @brief Remove process from the team.
/// @param process_id process slot inside team.
/// @retval true process was removed.
/// @retval false process doesn't exist in team.

/***********************************************************************************/

Void UserProcessScheduler::Remove(ProcessID process_id) {
  if (process_id < 0 || process_id > kCPSProcessLimitPerTeam) return;
  if (this->mTeam.mProcessList[process_id].Status == ProcessStatusKind::kInvalid) return;

  mTeam.mProcessList[process_id].Exit(kErrorSuccess);
}

/// @brief Is it a user scheduler?

Bool UserProcessScheduler::IsUser() {
  return Yes;
}

/// @brief Is it a kernel scheduler?

Bool UserProcessScheduler::IsKernel() {
  return No;
}

/// @brief Is it a SMP scheduler?

Bool UserProcessScheduler::HasMP() {
  MUST_PASS(kHandoverHeader);
  return kHandoverHeader->f_HardwareTables.f_MultiProcessingEnabled;
}

/***********************************************************************************/
/// @brief Run User scheduler object.
/// @return UserProcess count executed within a team.
/***********************************************************************************/

SizeT UserProcessScheduler::Run() {
  STATIC SizeT process_index{};  //! we store this guy to tell the scheduler how many
                                 //! things we have scheduled.

  UserProcessTeam& team  = mTeam;
  SizeT            limit = team.AsArray().Capacity();

  if (team.mProcessCur < 1) {
    return {};
  }

  if (process_index > limit)
    process_index = 0UL;
  else
    ++process_index;

  auto& process = team.AsArray()[process_index];

  //! Check if the process needs to be run.
  if (UserProcessHelper::CanBeScheduled(process)) {
    kout << process.Name << " will be scheduled to run...\r";

    //! Increase the usage time of the process.
    if (process.UTime < process.PTime) {
      ++process.UTime;
    }

    //! boost priority for processes that slept (interactive boost)
    if (process.STime > 0) {
      // the longer it slept, the bigger the boost (capped at kVeryHigh level)
      ProcessTime boost = process.STime / 10;
      if (boost > (Int32) AffinityKind::kHigh) boost = (Int32) AffinityKind::kHigh;
      process.PTime += boost;
      process.STime = 0;  // reset sleep counter after boost
    }

    this->TheCurrentProcess() = process;

    if (UserProcessHelper::Switch(process.StackFrame, process.ProcessId)) {
      // We add a bigger cooldown according to the RTime and affinity here.
      if (process.PTime < process.RTime && AffinityKind::kUltraHigh != process.Affinity) {
        if (process.RTime < (Int32) AffinityKind::kVeryHigh)
          process.RTime += (Int32) AffinityKind::kLowUsage;
        else if (process.RTime < (Int32) AffinityKind::kHigh)
          process.RTime += (Int32) AffinityKind::kStandard;
        else if (process.RTime < (Int32) AffinityKind::kStandard)
          process.RTime += (Int32) AffinityKind::kHigh;

        process.PTime -= process.RTime;
        process.RTime = 0UL;
      } else if (AffinityKind::kUltraHigh != process.Affinity) {
        process.PTime += (Int32) AffinityKind::kUltraHigh;
      }
    }
  } else {
    //! track sleep time for processes that are blocked/waiting
    if (process.Status == ProcessStatusKind::kFrozen) {
      ++process.STime;
    }
    ++process.RTime;
    --process.PTime;
  }

  return process_index;
}

/// @brief Gets the current scheduled team.
/// @return
UserProcessTeam& UserProcessScheduler::TheCurrentTeam() {
  return mTeam;
}

/***********************************************************************************/
/// @brief Switches the current team.
/// @param team the new team to switch to.
/// @retval true team was switched.
/// @retval false team was not switched.
/***********************************************************************************/

BOOL UserProcessScheduler::SwitchTeam(UserProcessTeam& team) {
  if (team.AsArray().Count() < 1ULL) return No;

  STATIC std::atomic_flag kLocked = ATOMIC_FLAG_INIT;

  while (kLocked.test_and_set(std::memory_order_acquire));

  this->mTeam = team;

  kLocked.clear(std::memory_order_release);

  return Yes;
}

/// @brief Gets current running process.
/// @return
UserProcess& UserProcessScheduler::TheCurrentProcess() {
  return mTeam.mCurrentProcess.Leak();
}

/// @brief Current proccess id getter.
/// @return UserProcess ID integer.
ErrorOr<ProcessID> UserProcessHelper::TheCurrentPID() {
  if (!UserProcessScheduler::The().TheCurrentProcess())
    return ErrorOr<ProcessID>{-kErrorProcessFault};

  kout << "UserProcessHelper::TheCurrentPID: Leaking ProcessId...\r";
  return ErrorOr<ProcessID>{UserProcessScheduler::The().TheCurrentProcess().ProcessId};
}

/// @brief Check if process can be schedulded.
/// @param process the process reference.
/// @retval true can be schedulded.
/// @retval false cannot be schedulded.
Bool UserProcessHelper::CanBeScheduled(const UserProcess& process) {
  if (process.Affinity == AffinityKind::kUltraHigh) return Yes;

  if (process.Status != ProcessStatusKind::kRunning) return No;
  if (process.Affinity == AffinityKind::kInvalid) return No;
  if (process.StackSize > kCPSMaxStackSz) return No;
  if (!process.Name[0]) return No;
  if (process.Signal.SignalID == sig_generate_unique<SIGTRAP>()) return No;

  return process.PTime < 1;
}

/***********************************************************************************/
/**
 * @brief Start scheduling the current team.
 */
/***********************************************************************************/

SizeT UserProcessHelper::StartScheduling() {
  return UserProcessScheduler::The().Run();
}

/***********************************************************************************/
/**
 * \brief Does a context switch in a CPU.
 * \param the_stack the stackframe of the running app.
 * \param new_pid the process's ProcessID.
 */
/***********************************************************************************/

Bool UserProcessHelper::Switch(HAL::StackFramePtr frame_ptr, ProcessID new_pid) {
  (Void)(kout << "IP: " << hex_number(frame_ptr->IP) << kendl);

  for (SizeT index{}; index < HardwareThreadScheduler::The().Capacity(); ++index) {
    if (!HardwareThreadScheduler::The()[index].Leak()) continue;

    if (HardwareThreadScheduler::The()[index].Leak()->Kind() == ThreadKind::kAPInvalid ||
        HardwareThreadScheduler::The()[index].Leak()->Kind() == ThreadKind::kAPBoot)
      continue;

    (Void)(kout << "AP_" << hex_number(index));

    if (HardwareThreadScheduler::The()[index].Leak()->IsBusy()) {
      kout << ": is busy\r";

      continue;
    }

    (Void)(kout << kendl);

    kout << ": is now trying to run a new task!\r";

    ////////////////////////////////////////////////////////////
    ///	Prepare task switch.								 ///
    ////////////////////////////////////////////////////////////

    HardwareThreadScheduler::The()[index].Leak()->Busy(YES);

    Bool ret{HardwareThreadScheduler::The()[index].Leak()->Switch(frame_ptr)};

    ////////////////////////////////////////////////////////////
    ///	Rollback on fail.    							    ///
    ////////////////////////////////////////////////////////////

    if (!ret) continue;

    (Void)(kout << "AP_" << hex_number(index));
    kout << " is now running a new task!\r";

    UserProcessHelper::TheCurrentPID().Leak().Leak() = new_pid;

    HardwareThreadScheduler::The()[index].Leak()->fPTime =
        UserProcessScheduler::The().TheCurrentTeam().AsArray()[new_pid].PTime;

    return YES;
  }

  kout << "Couldn't find a suitable core for the current process!\r";

  return NO;
}

////////////////////////////////////////////////////////////
/// @brief this checks if any process is on the team.
////////////////////////////////////////////////////////////
UserProcessScheduler::operator bool() {
  for (SizeT process_index = 0UL; process_index < mTeam.AsArray().Count(); ++process_index) {
    UserProcess& process = mTeam.AsArray()[process_index];
    if (UserProcessHelper::CanBeScheduled(process)) return true;
  }

  return false;
}

////////////////////////////////////////////////////////////
/// @brief this checks if no process is on the team.
////////////////////////////////////////////////////////////
Bool UserProcessScheduler::operator!() {
  SizeT cnt = 0UL;

  for (SizeT process_index = 0UL; process_index < mTeam.AsArray().Count(); ++process_index) {
    UserProcess& process = mTeam.AsArray()[process_index];
    if (UserProcessHelper::CanBeScheduled(process)) ++cnt;
  }

  return cnt == 0L;
}

}  // namespace Ne::Kernel

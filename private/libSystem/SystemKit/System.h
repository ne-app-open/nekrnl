// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#ifndef SYSTEMKIT_SYSTEM_H
#define SYSTEMKIT_SYSTEM_H

#include <SystemKit/Macros.h>

/// @brief TTY device path.
#define kPrintDevicePath "/devices/tty{}{}"

/// @brief Secure TTY device path.
#define kSecurePrintDevicePath "/devices/stty{}{}"

/// @brief Laser-disc path (Blu-ray, DVD, CD, etc.)
#define kCDDevicePath "/devices/dvd{}{}"

// ------------------------------------------------------------------------------------------ //
/// @brief Types API.
// ------------------------------------------------------------------------------------------ //

/// @brief Reference type, used for all references in the system, such as file descriptors, dylib
/// handles, thread handles, etc.
struct REF_TYPE _FINAL {
  UInt64  __hash;  /// @brief Hash of the syscall
  VoidPtr __self;  /// @brief Syscall self value.
};

typedef REF_TYPE* Ref;

typedef Ref IORef;
typedef Ref FSRef;
typedef Ref DylibRef;
typedef Ref ThreadRef;
typedef Ref SocketRef;
typedef Ref NetworkRef;
typedef Ref MutexRef;
typedef Ref EventRef;
typedef Ref SemaphoreRef;

// ------------------------------------------------------------------------------------------ //
/// @brief Dynamic Loader API.
// ------------------------------------------------------------------------------------------ //

/// @brief Get function which is part of the Dylib.
/// @param symbol the symbol to look for
/// @param dll_handle the Dylib handle.
/// @return the proc pointer.
IMPORT_C VoidPtr LdrGetDylibSymbolFromHandle(_Input const Char* symbol, _Input Ref dll_handle);

#ifndef LdrGetDylibSymbol
#define LdrGetDylibSymbol LdrGetDylibSymbolFromHandle
#endif

/// @brief Open Dylib handle.
/// @param path dll path.
/// @param drv driver letter.
/// @return a dylib ref.
IMPORT_C Ref LdrOpenDylibHandle(_Input const Char* path, _Input const Char* drive_letter);

/// @brief Close Dylib handle
/// @param dll_handle the dylib ref.
/// @return whether it closed or not.
IMPORT_C UInt32 LdrCloseDylibHandle(_Input Ref* dll_handle);

// ------------------------------------------------------------------------------------------ //
// File API.
// ------------------------------------------------------------------------------------------ //

/// @brief Opens a file from a drive.
/// @param fs_path the filesystem path.
/// @param drive_letter drive name, use NULL to use default drive location.
/// @return the file descriptor of the file.
IMPORT_C Ref IoOpenFile(const Char* fs_path, const Char* drive_letter);

/// @brief Closes a file and flushes its content.
/// @param file_desc the file descriptor.
/// @return Function doesn't return a type.
IMPORT_C Void IoCloseFile(_Input Ref file_desc);

/// @brief I/O control (ioctl) on a file.
/// @param file_desc the file descriptor.
/// @param ioctl_code the ioctl code.
/// @param in_data the input data.
/// @param out_data the output data.
/// @return the number of bytes written.
/// @note This function is used to control the file descriptor, introduced for OpenHeFS.
IMPORT_C SInt32 IoCtrlFile(_Input Ref file_desc, _Input UInt32 ioctl_code, _Input VoidPtr in_data,
                           _Output VoidPtr out_data);

/// @brief Gets the file mime (if any)
/// @param file_desc the file descriptor.
IMPORT_C const Char* IoMimeFile(_Input Ref file_desc);

/// @brief Gets the dir DIM.
/// @param dir_desc directory descriptor.
/// @note only works in OpenHeFS, will return nil-x/nil if used on any other filesystem.
IMPORT_C const Char* IoDimFile(_Input Ref dir_desc);

/// @brief Write data to a file ref
/// @param file_desc the file descriptor.
/// @param out_data the data to write.
/// @param sz_data the size of the data to write.
/// @return the number of bytes written.
IMPORT_C UInt32 IoWriteFile(_Input Ref file_desc, _Output VoidPtr out_data, UInt64 sz_data);

/// @brief Read data from a file.
/// @param file_desc the file descriptor.
/// @param out_data the data to read.
/// @param sz_data the size of the data to read.
IMPORT_C UInt32 IoReadFile(_Input Ref file_desc, _Output VoidPtr* out_data, UInt64 sz_data);

/// @brief Read data from a file.
/// @param file_desc the file descriptor.
/// @param out_data the data to read.
/// @param sz_data the size of the data to read.
IMPORT_C UInt32 IoReadFileII(_Input Ref file_desc, _Output VoidPtr out_data, UInt64 sz_data);

/// @brief Rewind the file pointer to the beginning of the file.
/// @param file_desc the file descriptor.
/// @return the number of bytes read.
IMPORT_C UInt64 IoRewindFile(_Input Ref file_desc);

/// @brief Tell the current position of the file pointer.
/// @param file_desc the file descriptor.
/// @return the current position of the file pointer.
IMPORT_C UInt64 IoTellFile(_Input Ref file_desc);

/// @brief Seek file offset from file descriptor.
IMPORT_C UInt64 IoSeekFile(_Input Ref file_desc, UInt64 file_offset);

// ------------------------------------------------------------------------
/// @brief NeKrnl Process API.
// ------------------------------------------------------------------------

/// @brief Spawns a Thread Information Block and Global Information Block inside the current
/// process.
/// @param process_id Target Process ID, must be valid.
/// @return > 0 error occurred or already present, = 0 success.
IMPORT_C UInt32 RtlSpawnIB(UIntPtr process_id);

/// @brief Spawns a process with a unique pid (stored as UIntPtr).
/// @param process_path process filesystem path.
/// @return > 0 process was created.
IMPORT_C UIntPtr RtlSpawnProcess(const Char* process_path, UInt32 argc, Char** argv, Char** envp,
                                 UInt32 envp_len);

/// @brief Exits a process with an exit_code.
/// @return if it has succeeded true, otherwise false.
IMPORT_C Bool RtlExitProcess(UIntPtr handle, UIntPtr exit_code);

// ------------------------------------------------------------------------
// Memory Manager API.
// ------------------------------------------------------------------------

/// @brief Creates a new heap from the process's address space.
/// @param len the length of it.
/// @param flags the flags of it.
/// @return heap pointer.
IMPORT_C VoidPtr MmCreateHeap(_Input UInt64 len, _Input UInt32 flags);

/// @brief Destroys the pointer
/// @param heap the heap itself.
/// @return void.
IMPORT_C SInt32 MmDestroyHeap(_Input VoidPtr heap);

/// @brief Change protection flags of a memory region.
IMPORT_C SInt32 MmSetHeapFlags(_Input VoidPtr heap, _Input UInt32 flags);

/// @brief Change protection flags of a memory region.
IMPORT_C UInt32 MmGetHeapFlags(_Input VoidPtr heap);

/// @brief Fill memory region with CRC32.
IMPORT_C UInt32 MmFillCRC32Heap(_Input VoidPtr heap);

/// @brief Copy memory region.
IMPORT_C VoidPtr MmCopyMemory(_Input VoidPtr dest, _Input VoidPtr src, _Input UInt64 len);

/// @brief Compare memory regions.
IMPORT_C SInt64 MmCmpMemory(_Input VoidPtr dest, _Input VoidPtr src, _Input UInt64 len);

/// @brief Fill memory region.
IMPORT_C VoidPtr MmFillMemory(_Input VoidPtr dest, _Input UInt64 len, _Input UInt8 value);

/// @brief Compare string regions.
IMPORT_C SInt64 MmStrCmp(_Input const Char* dest, _Input const Char* src);

/// @brief Get length of string.
IMPORT_C SInt64 MmStrLen(const Char* str);

// ------------------------------------------------------------------------
// @brief Error API.
// ------------------------------------------------------------------------

IMPORT_C SInt32 ErrGetLastError(Void);

// ------------------------------------------------------------------------
// @brief Threading API.
// ------------------------------------------------------------------------

/// @brief Exit the current thread.
/// @param exit_code the exit code.
IMPORT_C SInt32 ThrExitCurrentThread(_Input SInt32 exit_code);

/// @brief Exit the main thread.
/// @param exit_code the exit code.
IMPORT_C SInt32 ThrExitMainThread(_Input SInt32 exit_code);

/// @brief Exit a thread.
/// @param thread the thread to exit.
/// @param exit_code the exit code.
IMPORT_C SInt32 ThrExitThread(_Input ThreadRef thread, _Input SInt32 exit_code);

/// @brief Thread procedure function type.
typedef SInt32 (*ThrProcKind)(SInt32 argc, Char** argv);

/// @brief Creates a thread.
/// @param procedure the thread procedure.
/// @param argument_count number of arguments inside that thread.
/// @param flags Thread flags.
/// @return the thread object.
IMPORT_C ThreadRef ThrCreateThread(const Char* thread_name, ThrProcKind procedure,
                                   SInt32 argument_count, VoidPtr args, SInt32 flags);

/// @brief Yields the current thread.
/// @param thread the thread to yield.
IMPORT_C SInt32 ThrYieldThread(ThreadRef thrd);

/// @brief Get the current thread's ID.
IMPORT_C ThreadRef ThrCurrentThread(Void);

/// @brief Joins a thread.
/// @param thread the thread to join.
IMPORT_C SInt32 ThrJoinThread(ThreadRef thrd);

/// @brief Detach a thread.
/// @param thread the thread to detach.
IMPORT_C SInt32 ThrDetachThread(ThreadRef thrd);

// ------------------------------------------------------------------------
// @brief Drive Management API.
// ------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------ //
/// @brief Get the default drive letter.
/// @param void.
/// @return the drive letter.
// ------------------------------------------------------------------------------------------ //
IMPORT_C Char* DrvGetDefaultDriveLetter(Void);

// ------------------------------------------------------------------------------------------ //
/// @brief Get the drive letter from a path.
/// @param path the path.
/// @return the drive letter.
// ------------------------------------------------------------------------------------------ //
IMPORT_C Char* DrvGetDriveLetterFromPath(_Input const Char* path);

// ------------------------------------------------------------------------------------------ //
/// @brief Get a mounted drive from a letter.
/// @param letter the letter (A..Z).
/// @return the drive object.
// ------------------------------------------------------------------------------------------ //
IMPORT_C Ref DrvGetMountedDrive(_Input Char letter);

// ------------------------------------------------------------------------------------------ //
/// @brief Mount a drive.
/// @param path the path to mount.
/// @param letter the letter to mount.
// ------------------------------------------------------------------------------------------ //
IMPORT_C Void DrvMountDrive(_Input const Char* path, _Input const Char* letter);

// ------------------------------------------------------------------------------------------ //
/// @brief Unmount a drive.
/// @param letter the letter to unmount.
// ------------------------------------------------------------------------------------------ //
IMPORT_C Void DrvUnmountDrive(_Input Char letter);

// ------------------------------------------------------------------------
// Event handling API, use to listen to OS specific events.
// ------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------ //
/// @brief Add an event listener.
/// @param event_name the event name.
/// @param listener the listener to add.
/// @return the event listener.
// ------------------------------------------------------------------------------------------ //

IMPORT_C Void EvtAddListener(_Input const Char* event_name, _Input Ref listener);

// ------------------------------------------------------------------------------------------ //
/// @brief Remove an event listener.
/// @param event_name the event name.
/// @param listener the listener to remove.
/// @return the event listener.
// ------------------------------------------------------------------------------------------ //

IMPORT_C Void EvtRemoveListener(_Input const Char* event_name, _Input Ref listener);

// ------------------------------------------------------------------------------------------ //
/// @brief Dispatch an event.
/// @param event_name the event name.
/// @param event_data the event data.
/// @return the event data.
// ------------------------------------------------------------------------------------------ //

IMPORT_C VoidPtr EvtDispatchEvent(_Input const Char* event_name, _Input VoidPtr event_data);

// ------------------------------------------------------------------------------------------ //
// Power API.
// ------------------------------------------------------------------------------------------ //

enum {
  kPowerCodeInvalid  = 0,
  kPowerCodeShutdown = 12,
  kPowerCodeReboot,
  kPowerCodeSleep,
  kPowerCodeWake,
  kPowerCodeCount = kPowerCodeWake - kPowerCodeShutdown + 1,
};

IMPORT_C SInt32 PwrReadCode(_Output SInt32 code);

IMPORT_C SInt32 PwrSendCode(_Output SInt32 code);

// ------------------------------------------------------------------------------------------ //
// CD-ROM API.
// ------------------------------------------------------------------------------------------ //

IMPORT_C IORef CdOpenTray(Void);

IMPORT_C SInt32 CdEjectDrive(_Input IORef cdrom);

IMPORT_C SInt32 CdCloseTray(Void);

// ------------------------------------------------------------------------------------------ //
// TTY API.
// ------------------------------------------------------------------------------------------ //

IMPORT_C SInt32 PrintOut(IORef file /* nullptr to direct to stdout */, const Char* fmt, ...);

IMPORT_C SInt32 PrintIn(IORef file /* nullptr to direct to stdout */, const Char* fmt, ...);

IMPORT_C IORef PrintCreate(Void);

IMPORT_C UInt64 PrintSize(IORef ref);

IMPORT_C SInt32 PrintRelease(IORef ref);

IMPORT_C IORef PrintGet(const Char* path);

// ------------------------------------------------------------------------------------------ //
// @brief Scheduler/Debug API.
// ------------------------------------------------------------------------------------------ //

typedef SInt32 AffinityRef;
typedef UInt64 ProcessRef;

IMPORT_C SInt32 SchedSetAffinity(_Input ProcessRef, SInt32 req, _Input AffinityRef*);

IMPORT_C SInt32 SchedGetAffinity(_Input ProcessRef, _InOut AffinityRef*);

IMPORT_C SInt32 SchedFireSignal(_Input ProcessRef, SInt32);

IMPORT_C SInt32 SchedReadMemory(_Input ProcessRef, SInt32, SInt32);

IMPORT_C SInt32 SchedWriteMemory(_Input ProcessRef, SInt32, SInt32);

IMPORT_C UIntPtr SchedGetCurrentProcessID(Void);

// ------------------------------------------------------------------------------------------ //
// @brief Filesystem API.
// ------------------------------------------------------------------------------------------ //

IMPORT_C BOOL FsCopy(const Char* path, const Char* dst);

IMPORT_C BOOL FsMove(const Char* path, const Char* dst);

IMPORT_C BOOL FsExists(const Char* path);

IMPORT_C BOOL FsCreateDir(const Char* path);

IMPORT_C BOOL FsCreateFile(const Char* path);

IMPORT_C BOOL FsCreateAlias(const Char* path, const Char* from);

// ------------------------------------------------------------------------------------------ //
// @brief Installable Filesystem API.
// ------------------------------------------------------------------------------------------ //

IMPORT_C BOOL IfsMount(const Char* path, const Char* drive_letter);

IMPORT_C BOOL IfsUnmount(const Char* drive_letter);

IMPORT_C BOOL IfsIsMounted(const Char* drive_letter);

// ------------------------------------------------------------------------------------------ //
// @brief String Manip API.
// ------------------------------------------------------------------------------------------ //

IMPORT_C Char* StrFmt(const Char* fmt, ...);

IMPORT_C UInt64 StrMathToNumber(const Char* in, const Char** endp, const SInt16 base);

// ------------------------------------------------------------------------------------------ //
// @brief Semaphore API.
// ------------------------------------------------------------------------------------------ //

/// @brief Create a semaphore.
IMPORT_C _Output SemaphoreRef SemCreate(_Input UInt32 initial_count, _Input UInt32 max_count,
                                        _Input const Char* name);

/// @brief Wait on a semaphore.
IMPORT_C SInt32 SemWait(_Input SemaphoreRef sem);

/// @brief Close a semaphore.
IMPORT_C SInt32 SemClose(_Input SemaphoreRef sem);

// ------------------------------------------------------------------------------------------ //
// @brief User AgeAuth API.
// ------------------------------------------------------------------------------------------ //

#ifndef UserIsAdult
#define UserIsAdult UserIsOfAdultAge
#endif

IMPORT_C Bool UserIsOfAdultAge(_Input SInt32 uid);

enum {
    kAccountInvalid,
    kAccountChild = 0x67,
    kAccountTeen,
    kAccountAdult,
    kAccountTypeAgeCount = 3,
};

IMPORT_C SInt32 UserGetAccountAgeType(_Input SInt32 uid);

#endif  // ifndef SYSTEMKIT_SYSTEM_H

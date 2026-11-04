/*
 * Copyright (c) 2026 Amlal El Mahrouss for Ne.app.
 */

#include <SystemKit/System.h>

#ifndef PING_MAG
#define PING_MAG 'PING'
#endif

SInt32 _NeMain(SInt32 argc, Char* argv[]) {
  LIBSYS_UNUSED(argc);

  SocketRef pingSock;

  if (argc < 2) {
    PrintOut(nullptr, "HELP: ping <hostname>\n");
    return EXIT_FAILURE;
  }

  pingSock = IoOpenFile(argv[1], "\\mnt\\net");

  PrintOut(nullptr, "Pinging %s...\n", argv[1]);

  for (SizeT i = 0U; i < 4; ++i) {
    SInt32 result = 0;

    if (result != 0) {
      PrintOut(nullptr, "Request timed out.\n");
      return EXIT_FAILURE;
    }

    SInt32 send_byte = PING_MAG;
    
    if (!IoWriteFile(pingSock, &send_byte, sizeof(send_byte))) {
      PrintOut(nullptr, "ping: failing...\n");
    } else {
      PrintOut(nullptr, "ping: sending...\n");
    }

    IoReadFileII(pingSock, &send_byte, sizeof(send_byte));
    
    if (send_byte == PING_MAG) {
      PrintOut(nullptr, "ping: response...\n");
    }
  }

  IoCloseFile(pingSock);

  return EXIT_SUCCESS;
}

// EOF

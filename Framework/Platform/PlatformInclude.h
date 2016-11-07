#pragma once

// for orbis
#include <stdlib.h>

#include <system_param.h>
#include <system_service.h>

#include <arpa/inet.h>
#include <errno.h>
#include <kernel.h>
#include <sched.h>
#include <mspace.h>
#include <sys/mman.h>
#include <net.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <libnetctl.h>
#include <new>
#include <pad.h>
#include <sceerror.h>
#include <semaphore.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <user_service.h>
#include <libdbg.h>
#include <app_content.h>
#include <float.h>
#include <unordered_map>

#include <gnm.h>
#include <gnmx.h>

#include <video_out.h>
#include <gnmx/shader_parser.h>

#ifdef _MSC_VER
#             pragma warning(push)
#             pragma warning(disable:4996) // use deprecated class
#elif defined(__ORBIS__)
#             pragma clang diagnostic push
#             pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

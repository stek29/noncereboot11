#include <stdio.h>

#include "kutils.h"
#include "finder.h"
#include "offsets.h"
#include "debug.h"

uint64_t task_self_addr(void) {
    // can be done by:
    // - finding kernel base
    // - parsing kernel header
    // - using patchfinder to find kernel_task
    // - walking task list to find one with 
    //   task->bsd_info.pid == getpid()

    // or by using some other way to leak port
    // and leaking mach_task_self

    // I'd stick to second way because it's easier
    // to use on iOS 11.1.2

    uint64_t cached = 0;

    if (cached == 0) {
        uint64_t find_port_2(mach_port_t port, int disposition);
        cached = find_port_2(mach_task_self(), MACH_MSG_TYPE_COPY_SEND);
        DEBUG("task_self_addr: 0x%llx", cached);
    }

    return cached;
}

// from Ian Beer's find_port.c
uint64_t find_port_address(mach_port_name_t port) {
    uint64_t task_port_addr = task_self_addr();

    uint64_t task_addr = rk64(task_port_addr + OFF_IPC_PORT__IP_KOBJECT);
    uint64_t itk_space = rk64(task_addr + OFF_TASK__ITK_SPACE);
    uint64_t is_table = rk64(itk_space + OFF_IPC_SPACE__IS_TABLE);

    uint32_t port_index = port >> 8;
    uint64_t port_addr = rk64(is_table + (port_index * SIZ_IPC_ENTRY_T));
    return port_addr;
}

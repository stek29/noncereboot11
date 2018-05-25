// iOS 11 moves OFVariables to const
// https://twitter.com/s1guza/status/908790514178301952
// however, if we:
//  1) Can find IODTNVRAM service
//  2) Have tfp0 / kernel read|write|alloc
//  3) Can leak kernel address of mach port
// then we can fake vtable on IODTNVRAM object
// async_wake satisfies those requirements
// however, I wasn't able to actually set or get ANY nvram variable
// not even userread/userwrite
// Guess sandboxing won't let to access nvram

#include <stdlib.h>
#include <CoreFoundation/CoreFoundation.h>
#include "kutils.h"
#include "finder.h"
#include "offsets.h"
#include "debug.h"

// convertPropToObject calls getOFVariableType
// open convertPropToObject, look for first vtable call -- that'd be getOFVariableType
// find xrefs, figure out vtable start from that
// following are offsets of entries in vtable

typedef mach_port_t io_service_t;
typedef mach_port_t io_connect_t;
extern const mach_port_t kIOMasterPortDefault;
CFMutableDictionaryRef IOServiceMatching(const char *name) CF_RETURNS_RETAINED;
io_service_t IOServiceGetMatchingService(mach_port_t masterPort, CFDictionaryRef matching CF_RELEASES_ARGUMENT);

// get kernel address of IODTNVRAM object
uint64_t get_iodtnvram_obj(void) {
    static uint64_t IODTNVRAMObj = 0;
    
    if (IODTNVRAMObj == 0) {
        io_service_t IODTNVRAMSrv = IOServiceGetMatchingService(kIOMasterPortDefault, IOServiceMatching("IODTNVRAM"));
        if (!MACH_PORT_VALID(IODTNVRAMSrv)) {
            ERROR("Failed to get IODTNVRAM service");
            return 0;
        }
        uint64_t nvram_up = find_port_address(IODTNVRAMSrv);
        IODTNVRAMObj = rk64(nvram_up + OFF_IPC_PORT__IP_KOBJECT);

        DEBUG("IODTNVRAM obj at 0x%llx", IODTNVRAMObj);
    }

    return IODTNVRAMObj;
}

uint64_t orig_vtable = -1;

int unlocknvram(void) {
    uint64_t obj = get_iodtnvram_obj();
    if (obj == 0) {
        ERROR("get_iodtnvram_obj failed!");
        return 1;
    }

    uint64_t vtable_start = rk64(obj);

    orig_vtable = vtable_start;

    uint64_t vtable_end = vtable_start;
    // Is vtable really guaranteed to end with 0 or was it just a coincidence?..
    // should we just use some max value instead?
    while (rk64(vtable_end) != 0) vtable_end += sizeof(uint64_t);

    uint32_t vtable_len = (uint32_t) (vtable_end - vtable_start);

    // copy vtable to userspace
    uint64_t *buf = calloc(1, vtable_len);
    rkbuffer(vtable_start, buf, vtable_len);

    DEBUG("IODTNVRAM vtable: 0x%llx - 0x%llx", vtable_start, vtable_end);

    for (int i = 0; i != vtable_len; i += sizeof(uint64_t)) {
        DEBUG("\t[0x%03x]: 0x%llx", i, buf[i/sizeof(uint64_t)]);
    }

    // alter it
    buf[VTB_IODTNVRAM__GETOFVARIABLEPERM / sizeof(uint64_t)] = \
        buf[VTB_IODTNVRAM__SEARCHNVRAMPROPERTY / sizeof(uint64_t)];

    // allocate buffer in kernel and copy it back
    uint64_t fake_vtable = kalloc_wired(vtable_len);
    wkbuffer(fake_vtable, buf, vtable_len);

    // replace vtable on IODTNVRAM object
    wk64(obj, fake_vtable);

    free(buf);
    INFO("Unlocked nvram");
    return 0;
}

int locknvram(void) {
    if (orig_vtable == -1) {
        ERROR("Trying to lock nvram, but didnt unlock first");
        return -1;
    }

    uint64_t obj = get_iodtnvram_obj();
    if (obj == 0) { // would never happen but meh
        ERROR("get_iodtnvram_obj failed!");
        return 1;
    }
    
    wk64(obj, orig_vtable);

    INFO("Locked nvram");
    return 0;
}

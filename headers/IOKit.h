#ifndef IOKit_h
#define IOKit_h

#include <CoreFoundation/CoreFoundation.h>

typedef mach_port_t io_service_t;
typedef mach_port_t    io_object_t;
typedef io_object_t    io_registry_entry_t;
typedef char        io_name_t[128];
typedef char             io_struct_inband_t[4096];

extern const mach_port_t kIOMasterPortDefault;

io_service_t
IOServiceGetMatchingService(
                            mach_port_t  _masterPort,
                            CFDictionaryRef  matching);

CFMutableDictionaryRef
IOServiceMatching(
                  const char* name);

kern_return_t
IORegistryEntrySetCFProperties(
                               io_registry_entry_t    entry,
                               CFTypeRef         properties );
kern_return_t
IORegistryEntryGetProperty(
                           io_registry_entry_t    entry,
                           const io_name_t        propertyName,
                           io_struct_inband_t    buffer,
                           uint32_t          * size );

#endif /* IOKit_h */

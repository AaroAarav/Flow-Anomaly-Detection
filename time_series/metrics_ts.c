#include <stdio.h>
#include <time.h>
// Use the appropriate config header depending on which folder this file is in.
// If compiling in flow_specific, change this to "config_flow.h"
#include "config_ts.h" 

#ifdef _WIN32
    #include <windows.h>
    #include <psapi.h>
#else
    #include <sys/resource.h>
#endif

HardwareStats get_hardware_snapshot() {
    HardwareStats stats = {0};

    // 1. Latency (Wall Clock)
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    stats.timestamp_ms = (ts.tv_sec * 1000.0) + (ts.tv_nsec / 1000000.0);

#ifdef _WIN32
    HANDLE hProcess = GetCurrentProcess();
    
    // Force OS to drop inactive DLL pages from the Working Set before we take the snapshot.
    // This allows us to measure only the actual memory required by our algorithms.
    SetProcessWorkingSetSize(hProcess, (SIZE_T)-1, (SIZE_T)-1);
    
    // 2. RAM Usage
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
        stats.ram_kb = (long)(pmc.WorkingSetSize / 1024);
    }
    
    // 3. CPU Usage
    FILETIME ftCreation, ftExit, ftKernel, ftUser;
    if (GetProcessTimes(hProcess, &ftCreation, &ftExit, &ftKernel, &ftUser)) {
        ULARGE_INTEGER uli;
        uli.LowPart = ftUser.dwLowDateTime;
        uli.HighPart = ftUser.dwHighDateTime;
        stats.cpu_time_us = (long)(uli.QuadPart / 10); 
    }
    
    // 4. Disk I/O Usage
    IO_COUNTERS ioCounters;
    if (GetProcessIoCounters(hProcess, &ioCounters)) {
        stats.disk_io_bytes = (long)(ioCounters.ReadTransferCount + ioCounters.WriteTransferCount);
    }
#else
    // Linux/POSIX Fallbacks
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    stats.ram_kb = usage.ru_maxrss;
    stats.cpu_time_us = usage.ru_utime.tv_usec;
    stats.disk_io_bytes = usage.ru_inblock + usage.ru_oublock; 
#endif

    return stats;
}
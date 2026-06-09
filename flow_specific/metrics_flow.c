// File: metrics_flow.c
// Implements anomaly detection edge algorithms.

#include <stdio.h>
#include <time.h>
#include "config_flow.h" 

#ifdef _WIN32
    #include <windows.h>
    #include <psapi.h>
#else
    #include <sys/resource.h>
#endif

HardwareStats get_hardware_snapshot() {
    HardwareStats stats = {0};

    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    stats.timestamp_ms = (ts.tv_sec * 1000.0) + (ts.tv_nsec / 1000000.0);

#ifdef _WIN32
    HANDLE hProcess = GetCurrentProcess();
    
    SetProcessWorkingSetSize(hProcess, (SIZE_T)-1, (SIZE_T)-1);
    
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
        stats.ram_kb = (long)(pmc.WorkingSetSize / 1024);
    }
    
    FILETIME ftCreation, ftExit, ftKernel, ftUser;
    if (GetProcessTimes(hProcess, &ftCreation, &ftExit, &ftKernel, &ftUser)) {
        ULARGE_INTEGER uli;
        uli.LowPart = ftUser.dwLowDateTime;
        uli.HighPart = ftUser.dwHighDateTime;
        stats.cpu_time_us = (long)(uli.QuadPart / 10); 
    }
    
    IO_COUNTERS ioCounters;
    if (GetProcessIoCounters(hProcess, &ioCounters)) {
        stats.disk_io_bytes = (long)(ioCounters.ReadTransferCount + ioCounters.WriteTransferCount);
    }
#else
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    stats.ram_kb = usage.ru_maxrss;
    stats.cpu_time_us = usage.ru_utime.tv_usec;
    stats.disk_io_bytes = usage.ru_inblock + usage.ru_oublock; 
#endif

    return stats;
}
#include <stdio.h>
#include <time.h>

#ifdef _WIN32
    #include <windows.h>
    #include <psapi.h> // Windows specific process stats
#else
    #include <sys/resource.h>
#endif

void measure_ts_latency(const char* stage) {
    long max_rss = 0;
    long user_cpu = 0;

#ifdef _WIN32
    // Windows execution: Read RAM and CPU
    HANDLE hProcess = GetCurrentProcess();
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
        max_rss = (long)(pmc.PeakWorkingSetSize / 1024); // Convert Bytes to KB
    }
    
    FILETIME ftCreation, ftExit, ftKernel, ftUser;
    if (GetProcessTimes(hProcess, &ftCreation, &ftExit, &ftKernel, &ftUser)) {
        ULARGE_INTEGER uli;
        uli.LowPart = ftUser.dwLowDateTime;
        uli.HighPart = ftUser.dwHighDateTime;
        user_cpu = (long)(uli.QuadPart / 10); // Convert 100-nanosecond intervals to microseconds
    }
#else
    // Linux/Edge execution
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    max_rss = usage.ru_maxrss;
    user_cpu = usage.ru_utime.tv_usec;
#endif

    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    double timestamp = (ts.tv_sec * 1000.0) + (ts.tv_nsec / 1000000.0);

    FILE* log = fopen("../logs/ts_metrics.csv", "a");
    if(log) {
        // Output format: Timestamp, Stage, UserCPU_Microseconds, MaxRAM_KB
        fprintf(log, "%.2f,%s,%ld,%ld\n", timestamp, stage, user_cpu, max_rss);
        fclose(log);
    }
}
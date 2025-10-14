#pragma once

#define HOME_MENU "NPSX20001"

#define CTL_KERN 1
#define KERN_PROC 14
#define KERN_PROC_PROC 8

#define MiB(x) ((x) / (1024.0 * 1024))
#define GiB(x) ((x) / (1024.0 * 1024 * 1024))

#define STATFS_SYSCALL 396
#define SYS_thr_set_name 464

#define KI_NSPARE_INT 4
#define KI_NSPARE_LONG 12
#define KI_NSPARE_PTR 6
#define WMESGLEN 8
#define LOCKNAMELEN 8
#define TDNAMLEN 16
#define COMMLEN 19
#define KI_EMULNAMELEN 16
#define KI_NGROUPS 16
#define LOGNAMELEN 17
#define LOGINCLASSLEN 17

typedef uint32_t fixpt_t;
typedef uint64_t vm_size_t;

typedef int32_t lwpid_t;
typedef int64_t segsz_t;

extern int GOLDHEN_OFFSET;

extern "C"
{
    int orbis_syscall(int num, ...);
    int sysctlbyname(const char *name, void *oldp, size_t *oldlenp,
                     const void *newp, size_t newlen);

    int32_t sceKernelGetSystemSwVersion(OrbisKernelSwVersion *version);
    uint32_t sceKernelGetCpuTemperature(uint32_t *celsius);
    uint32_t sceKernelGetSocSensorTemperature(uint32_t *unknown, uint32_t *celsius);
    int32_t sceUserServiceGetForegroundUser(int32_t *userId);
    int32_t sceUserServiceGetUserName(int32_t userId, char *userName, size_t len);
    int32_t sceKernelGetAppInfo(pid_t pid, OrbisAppInfo *pInfo);

    void sceSysUtilSendSystemNotificationWithText(int type, const char *message);
}

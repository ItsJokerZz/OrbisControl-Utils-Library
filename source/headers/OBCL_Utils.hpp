#pragma once

#include "OBCL_Utils/includes.hpp"

extern "C"
{
#ifndef DEFINE_ONCE
#define DEFINE_ONCE

  __asm__(".att_syntax prefix\n"
          ".local orbis_syscall\n"
          ".type orbis_syscall,@function\n"
          "orbis_syscall:\n"
          "movq $0, %rax\n"
          "movq %rcx, %r10\n"
          "syscall\n"
          "jb 1f\n"
          "retq\n"
          "1:\n"
          "pushq %rax\n"
          "callq __error\n"
          "popq %rcx\n"
          "movl %ecx, 0(%rax)\n"
          "movq $0xFFFFFFFFFFFFFFFF, %rax\n"
          "movq $0xFFFFFFFFFFFFFFFF, %rdx\n"
          "retq\n");

#endif
}

enum class power_state : uint8_t
{
  DO_NOTHING,
  SUSPEND,
  SHUTDOWN,
  RESTART
};

enum class disk_info : uint8_t
{
  PERCENT_USED,
  TOTAL_SPACE,
  USED_SPACE,
  FREE_SPACE
};

struct ProcessInfo
{
  uint32_t pid;
  uint32_t ppid;
  uint32_t pgid;
  uint32_t sid;
  uint32_t uid;
  char state[16];
  uint16_t appId;
  char titleId[16];
  double memoryRSS_MiB;
  double memorySize_MiB;
  uint32_t uptimeSec;
  char command[256];

  ProcessInfo()
      : pid(0), ppid(0), pgid(0), sid(0), uid(0), appId(0),
        memoryRSS_MiB(0.0), memorySize_MiB(0.0), uptimeSec(0)
  {
    state[0] = '\0';
    titleId[0] = '\0';
    command[0] = '\0';
  }
};

struct ProcessTable
{
  std::map<uint32_t, ProcessInfo> byPid;

  void addProcess(const ProcessInfo &p) { byPid[p.pid] = p; }

  ProcessInfo *getProcess(uint32_t pid)
  {
    auto it = byPid.find(pid);
    return (it != byPid.end()) ? &it->second : nullptr;
  }

  const ProcessInfo *getProcess(uint32_t pid) const
  {
    auto it = byPid.find(pid);
    return (it != byPid.end()) ? &it->second : nullptr;
  }

  void clear() { byPid.clear(); }

  size_t size() const { return byPid.size(); }

  bool empty() const { return byPid.empty(); }
};

struct priority
{
  u_char pri_class;
  u_char pri_level;
  u_char pri_native;
  u_char pri_user;
};

struct rusage
{
  struct timeval ru_utime;
  struct timeval ru_stime;
  long ru_maxrss;
#define ru_first ru_ixrss
  long ru_ixrss;
  long ru_idrss;
  long ru_isrss;
  long ru_minflt;
  long ru_majflt;
  long ru_nswap;
  long ru_inblock;
  long ru_oublock;
  long ru_msgsnd;
  long ru_msgrcv;
  long ru_nsignals;
  long ru_nvcsw;
  long ru_nivcsw;
#define ru_last ru_nivcsw
};

struct kinfo_proc
{
  int ki_structsize;
  int ki_layout;
  struct pargs *ki_args;
  struct proc *ki_paddr;
  struct user *ki_addr;
  struct vnode *ki_tracep;
  struct vnode *ki_textvp;
  struct filedesc *ki_fd;
  struct vmspace *ki_vmspace;
  void *ki_wchan;
  pid_t ki_pid;
  pid_t ki_ppid;
  pid_t ki_pgid;
  pid_t ki_tpgid;
  pid_t ki_sid;
  pid_t ki_tsid;
  short ki_jobc;
  short ki_spare_short1;
  dev_t ki_tdev;
  sigset_t ki_siglist;
  sigset_t ki_sigmask;
  sigset_t ki_sigignore;
  sigset_t ki_sigcatch;
  uid_t ki_uid;
  uid_t ki_ruid;
  uid_t ki_svuid;
  gid_t ki_rgid;
  gid_t ki_svgid;
  short ki_ngroups;
  short ki_spare_short2;
  gid_t ki_groups[KI_NGROUPS];
  vm_size_t ki_size;
  segsz_t ki_rssize;
  segsz_t ki_swrss;
  segsz_t ki_tsize;
  segsz_t ki_dsize;
  segsz_t ki_ssize;
  u_short ki_xstat;
  u_short ki_acflag;
  fixpt_t ki_pctcpu;
  u_int ki_estcpu;
  u_int ki_slptime;
  u_int ki_swtime;
  u_int ki_cow;
  u_int64_t ki_runtime;
  struct timeval ki_start;
  struct timeval ki_childtime;
  long ki_flag;
  long ki_kiflag;
  int ki_traceflag;
  char ki_stat;
  signed char ki_nice;
  char ki_lock;
  char ki_rqindex;
  u_char ki_oncpu_old;
  u_char ki_lastcpu_old;
  char ki_tdname[TDNAMLEN + 1];
  char ki_wmesg[WMESGLEN + 1];
  char ki_login[LOGNAMELEN + 1];
  char ki_lockname[LOCKNAMELEN + 1];
  char ki_comm[COMMLEN + 1];
  char ki_emul[KI_EMULNAMELEN + 1];
  char ki_loginclass[LOGINCLASSLEN + 1];
  char ki_sparestrings[50];
  int ki_spareints[KI_NSPARE_INT];
  int ki_oncpu;
  int ki_lastcpu;
  int ki_tracer;
  int ki_flag2;
  int ki_fibnum;
  u_int ki_cr_flags;
  int ki_jid;
  int ki_numthreads;
  lwpid_t ki_tid;
  struct priority ki_pri;
  struct rusage ki_rusage;
  struct rusage ki_rusage_ch;
  struct pcb *ki_pcb;
  void *ki_kstack;
  void *ki_udata;
  struct thread *ki_tdaddr;
  void *ki_spareptrs[KI_NSPARE_PTR];
  long ki_sparelongs[KI_NSPARE_LONG];
  long ki_sflag;
  long ki_tdflags;
};

int sysctl(int *name, uint32_t namelen, void *oldp, size_t *oldlenp,
           const void *newp, size_t newlen);

namespace Process
{
  std::string find_name_of_pid(pid_t pid);
  int find_pid_by_name(std::string_view name);
  nlohmann::json get_list();
  void change_proc_name(std::string_view name);
}

namespace App
{
  std::string get_titleId();
  std::string get_name(std::string_view titleId = {});
  std::string parse_sfo_param(std::string_view key);
  std::string get_version();
  std::string get_minFW();
  std::string get_region();
  bool is_daemon_process();
  std::string find_exec_by_titleId();
  std::string get_info(std::string_view key);
}

namespace System
{
  bool check_for_goldhen();
  std::string get_local_ip();
  std::string get_name();
  std::string get_type();
  std::string get_fw_version();
  std::string get_disk_info(disk_info type);
  std::string get_uptime();
  std::string get_model();
  uint32_t get_cpu_temperature();
  uint32_t get_soc_temperature();
  void set_temperature_limit(uint8_t limit = 60);
  void set_power_state(power_state state = power_state::DO_NOTHING);
  void ring_buzzer(int type);
  std::string get_username();
  void text_notify(int type, std::string_view msg);
  void image_notify(std::string &iconUri, std::string_view text);
}

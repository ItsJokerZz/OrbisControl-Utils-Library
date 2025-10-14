#pragma once

namespace sys_proc
{
  enum class cmds : uint8_t
  {
    ALLOC = 1,
    FREE,
    PROTECT,
    VM_MAP,
    INSTALL,
    CALL,
    ELF,
    INFO,
    THRINFO,
    // PRX_LIST
  };

  namespace args
  {
#pragma pack(push, 1)

    struct list_entry
    {
      char p_comm[32];
      pid_t pid;
    };

    struct read_write
    {
      uint64_t address;
      void *data;
      uint64_t length;
      uint64_t write;
    };

    struct alloc_free
    {
      uint64_t address;
      uint64_t length;
    };

    struct protect
    {
      uint64_t address;
      uint64_t length;
      uint64_t prot;
    };

    struct vm_map_entry
    {
      char name[32];
      uint64_t start;
      uint64_t end;
      uint64_t offset;
      uint16_t prot;
    };

    struct vm_map_list
    {
      vm_map_entry *maps;
      uint64_t num;
    };

    struct install
    {
      uint64_t stubentryaddr;
    };

    struct call
    {
      pid_t pid;
      uint64_t rpcstub;
      uint64_t rax;
      uint64_t rip;
      uint64_t rdi;
      uint64_t rsi;
      uint64_t rdx;
      uint64_t rcx;
      uint64_t r8;
      uint64_t r9;
    };

    struct elf
    {
      void *elf;
      uint64_t entry;
    };

    struct info
    {
      pid_t pid;
      char name[40];
      char path[64];
      char titleid[16];
      char contentid[64];
    };

    struct thrinfo
    {
      uint32_t lwpid;
      uint32_t priority;
      char name[32];
    };

    /*
        struct prx_list_entry
        {
          uint32_t handle;
          char name[256];
          uint64_t text_address;
          uint32_t text_size;
          uint64_t data_address;
          uint32_t data_size;
        };

        struct prx_list
        {
          struct prx_list_entry *entries;
          uint64_t num;
        };
    */

#pragma pack(pop)
  }

  int list(sys_proc::args::list_entry *procs, uint64_t *num);

  int rw(sys_proc::args::read_write *args);
  int rw(pid_t pid, sys_proc::args::read_write *args);

  int cmd(cmds cmd, void *data);
  int cmd(pid_t pid, cmds cmd, void *data);

  int alloc(sys_proc::args::alloc_free *args, bool free);
  int alloc(pid_t pid, sys_proc::args::alloc_free *args, bool free);

  int protect(sys_proc::args::protect *args);
  int protect(pid_t pid, sys_proc::args::protect *args);

  std::vector<sys_proc::args::vm_map_entry> vm_map();

  int install(sys_proc::args::install *args);
  int install(pid_t pid, sys_proc::args::install *args);

  int call(sys_proc::args::call *args);
  int call(pid_t pid, sys_proc::args::call *args);

  /*
    int elf();
    int info();
    int thrinfo();
  */

}

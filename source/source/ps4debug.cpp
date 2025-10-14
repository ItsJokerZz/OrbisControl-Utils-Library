#include "OBCL_Utils.hpp"

int GOLDHEN_OFFSET = -1;

namespace sys_proc
{
    int list(sys_proc::args::list_entry *procs, uint64_t *num)
    {
        return orbis_syscall(107 + GOLDHEN_OFFSET, procs, num);
    }

    int rw(sys_proc::args::read_write *args)
    {
        pid_t pid = std::stoi(App::get_info("pid"));
        return orbis_syscall(108 + GOLDHEN_OFFSET, pid, args->address,
                             args->data, args->length, args->write);
    }

    int rw(pid_t pid, sys_proc::args::read_write *args)
    {
        return orbis_syscall(108 + GOLDHEN_OFFSET, pid, args->address,
                             args->data, args->length, args->write);
    }

    int cmd(cmds cmd, void *data)
    {
        pid_t pid = std::stoi(App::get_info("pid"));
        return orbis_syscall(109 + GOLDHEN_OFFSET, pid, cmd, data);
    }

    int cmd(pid_t pid, cmds cmd, void *data)
    {
        return orbis_syscall(109 + GOLDHEN_OFFSET, pid, cmd, data);
    }

    int alloc(sys_proc::args::alloc_free *args, bool free)
    {
        if (!free)
            return cmd(sys_proc::cmds::ALLOC, args);
        else
            return cmd(sys_proc::cmds::FREE, args);
    }

    int alloc(pid_t pid, sys_proc::args::alloc_free *args, bool free)
    {
        if (!free)
            return cmd(pid, sys_proc::cmds::ALLOC, args);
        else
            return cmd(pid, sys_proc::cmds::FREE, args);
    }

    int protect(sys_proc::args::protect *args)
    {
        return cmd(sys_proc::cmds::PROTECT, args);
    }

    int protect(pid_t pid, sys_proc::args::protect *args)
    {
        return cmd(pid, sys_proc::cmds::PROTECT, args);
    }

    std::vector<sys_proc::args::vm_map_entry> vm_map()
    {
        sys_proc::args::vm_map_list args{nullptr, 0};

        if (cmd(sys_proc::cmds::VM_MAP, &args))
            return {};

        args.maps = new sys_proc::args::vm_map_entry[args.num];
        if (!args.maps)
            return {};

        if (cmd(sys_proc::cmds::VM_MAP, &args))
        {
            delete[] args.maps;
            return {};
        }

        std::vector<sys_proc::args::vm_map_entry> maps(args.maps, args.maps + args.num);
        delete[] args.maps;
        return maps;
    }

    int install(sys_proc::args::install *args)
    {
        return cmd(sys_proc::cmds::INSTALL, args);
    }

    int install(pid_t pid, sys_proc::args::install *args)
    {
        return cmd(pid, sys_proc::cmds::INSTALL, args);
    }

    int call(sys_proc::args::call *args)
    {
        args->pid = std::stoi(App::get_info("pid"));
        return cmd(sys_proc::cmds::CALL, args);
    }

    int call(pid_t pid, sys_proc::args::call *args)
    {
        args->pid = pid;
        return cmd(pid, sys_proc::cmds::CALL, args);
    }
}

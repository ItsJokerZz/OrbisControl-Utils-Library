#include "OBCL_Utils.hpp"

namespace Process
{
    std::string find_name_of_pid(pid_t target_pid)
    {
        sys_proc::args::list_entry *list;
        uint64_t num;

        if (sys_proc::list(nullptr, &num) || num == 0)
            return {};

        list = (sys_proc::args::list_entry *)malloc(num * sizeof(sys_proc::args::list_entry));
        if (!list)
            return {};

        if (sys_proc::list(list, &num))
        {
            free(list);
            return {};
        }

        std::string result;
        for (size_t i = 0; i < num; i++)
        {
            if (list[i].pid == target_pid)
            {
                result = list[i].p_comm;
                break;
            }
        }

        free(list);
        return result;
    }

    int find_pid_by_name(std::string_view name)
    {
        sys_proc::args::list_entry *list;
        uint64_t num;

        if (sys_proc::list(nullptr, &num) || num == 0)
            return -1;

        list = (sys_proc::args::list_entry *)malloc(num * sizeof(sys_proc::args::list_entry));
        if (!list)
            return -1;

        if (sys_proc::list(list, &num))
        {
            free(list);
            return -1;
        }

        for (size_t i = 0; i < num; i++)
        {
            if (strncmp(list[i].p_comm, name.data(), 32) == 0)
            {
                pid_t pid = list[i].pid;
                free(list);
                return pid;
            }
        }

        free(list);
        return -1;
    }

    nlohmann::json get_list()
    {
        int mib[4] = {CTL_KERN, KERN_PROC, KERN_PROC_PROC, 0};
        size_t buf_size;

        if (sysctl(mib, 4, NULL, &buf_size, NULL, 0) || !buf_size)
            return nlohmann::json::array();

        void *buf = malloc(buf_size);
        if (!buf)
            return nlohmann::json::array();

        if (sysctl(mib, 4, buf, &buf_size, NULL, 0))
        {
            free(buf);
            return nlohmann::json::array();
        }

        time_t now = time(NULL);
        const char *state_abbrev[] = {"", "START", "RUN", "SLEEP", "STOP", "ZOMB", "WAIT", "LOCK"};

        nlohmann::json response = nlohmann::json::array();

        for (uint8_t *ptr = (uint8_t *)buf; ptr < (uint8_t *)buf + buf_size;)
        {
            kinfo_proc *ki = (kinfo_proc *)ptr;
            ptr += ki->ki_structsize;

            OrbisAppInfo appinfo = {};
            sceKernelGetAppInfo(ki->ki_pid, &appinfo);

            nlohmann::json entry;
            entry["pid"] = ki->ki_pid;
            entry["ppid"] = ki->ki_ppid;
            entry["pgid"] = ki->ki_pgid;
            entry["sid"] = ki->ki_sid;
            entry["uid"] = ki->ki_uid;
            entry["state"] = state_abbrev[(int)ki->ki_stat];
            entry["appId"] = appinfo.AppId;
            entry["titleId"] = appinfo.TitleId;
            entry["memoryRSS_MiB"] = MiB(ki->ki_rssize * PAGE_SIZE);
            entry["memorySize_MiB"] = MiB(ki->ki_size);
            entry["uptimeSec"] = (uint32_t)(now - ki->ki_start.tv_sec);
            entry["command"] = find_name_of_pid(ki->ki_pid);

            response.push_back(entry);
        }

        free(buf);
        return response;
    }

    void change_proc_name(std::string_view name)
    {
        orbis_syscall(SYS_thr_set_name, -1, name);
    }
}

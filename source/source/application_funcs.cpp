#include "OBCL_Utils.hpp"

namespace App
{
    std::string get_titleId()
    {
        DIR *dir = opendir("/mnt/sandbox/");
        if (!dir)
            return HOME_MENU;

        std::string titleId;
        std::regex titleRegex("(?!NPXS)([A-Z0-9]{4}[0-9]{5})");
        struct dirent *entry;

        while ((entry = readdir(dir)) != nullptr)
        {
            std::string dirName(entry->d_name);
            std::smatch match;
            if (std::regex_search(dirName, match, titleRegex) && match.size() > 1)
            {
                std::string candidate = match.str(1);
                std::vector<std::string> paths = {
                    "/system_data/priv/appmeta/" + candidate + "/param.sfo",
                    "/system/vsh/app/" + candidate + "/sce_sys/param.sfo"};

                bool isDaemon = false;
                for (const auto &path : paths)
                {
                    FILE *file = fopen(path.c_str(), "rb");
                    if (!file)
                        continue;

                    fseek(file, 0, SEEK_END);
                    long fileSize = ftell(file);
                    fseek(file, 0, SEEK_SET);

                    std::vector<u8> sfoData(fileSize);
                    fread(sfoData.data(), 1, fileSize, file);
                    fclose(file);

                    SfoReader sfoReader(sfoData);
                    std::string category = sfoReader.GetValueFor<std::string>("CATEGORY");
                    if (category == "gdd")
                    {
                        isDaemon = true;
                        break;
                    }
                }

                if (isDaemon)
                    continue;

                titleId = candidate;
                break;
            }
        }

        closedir(dir);
        return titleId.empty() ? HOME_MENU : titleId;
    }

    std::string get_name(std::string_view titleId)
    {
        std::string _titleId;
        if (titleId.empty())
        {
            _titleId = get_titleId();
            titleId = _titleId;
        }

        if (titleId == HOME_MENU)
            return "User Interface (UI)";

        return parse_sfo_param("TITLE");
    }

    std::string parse_sfo_param(std::string_view key)
    {
        std::string titleId = get_titleId();

        std::vector<std::string> paths = {
            "/system_data/priv/appmeta/" + titleId + "/param.sfo",
            "/system/vsh/app/" + titleId + "/sce_sys/param.sfo",
            "/system_ex/app/" + titleId + "/sce_sys/param.sfo",
        };

        for (const auto &path : paths)
        {
            FILE *file = fopen(path.c_str(), "rb");
            if (!file)
                continue;

            fseek(file, 0, SEEK_END);
            long fileSize = ftell(file);
            fseek(file, 0, SEEK_SET);

            std::vector<u8> sfoData(fileSize);
            fread(sfoData.data(), 1, fileSize, file);
            fclose(file);

            SfoReader sfoReader(sfoData);
            return sfoReader.GetValueFor<std::string>(key.data());
        }

        printf("Failed to open param.sfo for %s", titleId.c_str());
        return "";
    }

    std::string get_version()
    {
        return parse_sfo_param("VERSION");
    }

    std::string get_minFW()
    {
        char versionString[10];
        std::string app_info = parse_sfo_param("PUBTOOLINFO");
        std::regex regex("sdk_ver=(\\d{8})");
        std::smatch match;

        if (std::regex_search(app_info, match, regex) && match.size() > 1)
        {
            std::string sdk_version = match.str(1);
            int major = std::stoi(sdk_version.substr(0, 2));
            int minor = std::stoi(sdk_version.substr(2, 2));

            snprintf(versionString, sizeof(versionString), "%02d.%02d", major, minor);

            return std::string(versionString);
        }

        return "";
    }

    std::string get_region()
    {
        char content_id[10];
        std::string app_info = parse_sfo_param("CONTENT_ID");
        std::regex regex("^(\\w{2})\\d+");
        std::smatch match;

        if (std::regex_search(app_info, match, regex) && match.size() > 1)
        {
            std::string prefix = match.str(1);
            std::string region;
            if (prefix == "UP")
                region = "USA";
            else if (prefix == "JP")
                region = "JAP";
            else if (prefix == "AS")
                region = "ASIA";
            else if (prefix == "EP")
                region = "EUR";

            return region;
        }

        return "";
    }

    bool is_daemon_process()
    {
        return get_titleId() == HOME_MENU ||
               parse_sfo_param("CATEGORY") == "gdd";
    }

    std::string find_exec_by_titleId()
    {
        std::string titleId = get_titleId();
        std::string path = "/mnt/sandbox/" + titleId + "_000/app0/";

        DIR *dir = opendir(path.c_str());
        if (!dir)
            return "";

        struct dirent *entry;
        while ((entry = readdir(dir)) != nullptr)
        {
            std::string fileName = std::string(entry->d_name).substr(0, strlen(entry->d_name));

            if (fileName.find(".bin") != std::string::npos || fileName.find(".elf") != std::string::npos)
            {
                int pid = Process::find_pid_by_name(fileName.c_str());

                if (pid <= 0)
                    continue;

                std::string procName = Process::find_name_of_pid(pid);
                if (!procName.empty())
                    procName = procName.substr(0, procName.find('\0'));

                closedir(dir);
                return procName;
            }
        }

        closedir(dir);
        return "";
    }

    std::string get_info(std::string_view key)
    {
        static const std::regex ps2Pattern(
            R"(^(SLES|SCES|SCED|SLUS|SCUS|SLPS|SCAJ|SLKA|SLPM|SCPS|CF00|SCKA|ALCH|CPCS|SLAJ|KOEI|ARZE|TCPS|SCCS|PAPX|SRPM|GUST|WLFD|ULKS|VUGJ|HAKU|ROSE|CZP2|ARP2|PKP2|SLPN|NMP2|MTP2|SCPM|PBPX)\d*$)");

        std::string titleId = get_titleId();
        bool is_home = (titleId == HOME_MENU);
        bool is_daemon = is_daemon_process();

        std::string execName = is_home ? "SceShellUI" : find_exec_by_titleId();
        int pid = Process::find_pid_by_name(execName);

        if (pid == -1)
            is_daemon = true;

        if (is_daemon)
        {
            is_home = true;
            titleId = HOME_MENU;
            execName = "SceShellUI";
        }

        std::string pidStr = std::to_string(pid);
        std::string execFile = is_home ? "eboot.bin" : execName;
        std::string minFW = is_home ? "" : get_minFW();
        std::string region = is_home ? "" : get_region();
        std::string version = is_home ? "" : get_version();
        std::string imagePath = is_home ? "" : "/user/appmeta/" + titleId + "/icon0.png";
        std::string name = get_name();

        std::string type;
        if (is_daemon)
            type = "DAEMON";
        else if (titleId.rfind("CUSA", 0) == 0)
            type = "PS4";
        else if (std::regex_match(titleId, ps2Pattern))
            type = "PS1/PS2";
        else
            type = "Homebrew";

        std::string uptime_formatted = "00:00:00:00";
        std::string memory_rss = "0 MB";

        if (pid > 0)
        {
            nlohmann::json proc_list = Process::get_list();

            for (auto &proc : proc_list)
            {
                if (proc["pid"].get<int>() == pid)
                {
                    uint32_t uptime_sec = proc["uptimeSec"].get<uint32_t>();
                    uint32_t days = uptime_sec / 86400;
                    uint32_t hours = (uptime_sec % 86400) / 3600;
                    uint32_t minutes = (uptime_sec % 3600) / 60;
                    uint32_t seconds = uptime_sec % 60;

                    char uptime_buf[12];
                    snprintf(uptime_buf, sizeof(uptime_buf), "%02u:%02u:%02u:%02u",
                             days, hours, minutes, seconds);
                    uptime_formatted = uptime_buf;

                    char memory_rss_buf[32];
                    snprintf(memory_rss_buf, sizeof(memory_rss_buf), "%.2f MB", proc["memoryRSS_MiB"].get<float>());
                    memory_rss = memory_rss_buf;

                    break;
                }
            }
        }

        if (key == "pid")
            return pidStr;
        if (key == "exec")
            return execFile;
        if (key == "minFW")
            return minFW;
        if (key == "type")
            return type;
        if (key == "titleId")
            return titleId;
        if (key == "name")
            return name;
        if (key == "region")
            return region;
        if (key == "version")
            return version;
        if (key == "image")
            return imagePath;
        if (key == "uptime")
            return uptime_formatted;
        if (key == "memory")
            return memory_rss;

        return {};
    }
}

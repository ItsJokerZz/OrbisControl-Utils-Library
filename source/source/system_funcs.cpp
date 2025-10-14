#include "OBCL_Utils.hpp"

int sysctl(int *name, uint32_t namelen, void *oldp, size_t *oldlenp, const void *newp, size_t newlen)
{
    return orbis_syscall(202, name, namelen, oldp, oldlenp, newp, newlen);
}

namespace System
{
    bool check_for_goldhen()
    {
        if (GOLDHEN_OFFSET == -1)
        {
            uint64_t tmp;
            if (orbis_syscall(107, NULL, &tmp) != 0)
                GOLDHEN_OFFSET = 90;
            else
                GOLDHEN_OFFSET = 0;
        }

        return GOLDHEN_OFFSET == 90;
    }

    std::string get_local_ip()
    {
        int sock = socket(PF_INET, SOCK_DGRAM, 0);
        if (sock == -1)
            return "";

        sockaddr_in remote_addr{};
        remote_addr.sin_family = AF_INET;
        remote_addr.sin_addr.s_addr = inet_addr("8.8.8.8");
        remote_addr.sin_port = htons(53);

        if (connect(sock, reinterpret_cast<sockaddr *>(&remote_addr),
                    sizeof(remote_addr)) == -1)
        {
            close(sock);
            return "";
        }

        sockaddr_in local_addr{};
        socklen_t addrlen = sizeof(local_addr);
        if (getsockname(sock, reinterpret_cast<sockaddr *>(&local_addr), &addrlen) ==
            -1)
        {
            close(sock);
            return "";
        }

        close(sock);

        char buf[INET_ADDRSTRLEN];
        if (inet_ntop(AF_INET, &local_addr.sin_addr, buf, INET_ADDRSTRLEN) == nullptr)
            return "";

        return std::string(buf);
    }

    std::string get_name()
    {
        char buffer[65]{};
        int ret = sceSystemServiceParamGetString(
            ORBIS_SYSTEM_SERVICE_PARAM_ID_SYSTEM_NAME, buffer, sizeof(buffer));
        return std::string(buffer);
    }

    std::string get_type()
    {
        if (sceKernelIsDevKit())
            return "DEVKIT";
        if (sceKernelIsTestKit())
            return "TESTKIT";
        return "RETAIL";
    }

    std::string get_fw_version()
    {
        static char versionString[32];
        OrbisKernelSwVersion versionInfo;

        if (sceKernelGetSystemSwVersion(&versionInfo) < 0)
            return nullptr;

        int major, minor;
        if (sscanf(versionInfo.VersionString, "%02d.%02d", &major, &minor) == 2)
            snprintf(versionString, sizeof(versionString), "%02d.%02d", major, minor);

        return versionString[0] ? versionString : nullptr;
    }

    std::string get_disk_info(disk_info type)
    {
        struct statfs s;
        const char *MOUNT_POINT = "/user";

        long blocks_used = 0, percentUsed = 0;
        double totalSpace = 0, usedSpace = 0, freeSpace = 0;

        if (orbis_syscall(STATFS_SYSCALL, MOUNT_POINT, &s) != 0)
        {
            printf("Cannot open %s", MOUNT_POINT);
            return "";
        }

        if (s.f_blocks > 0)
        {
            blocks_used = s.f_blocks - s.f_bfree;
            percentUsed = static_cast<long>(
                blocks_used * 100.0 / (blocks_used + s.f_bavail) + 0.5);
        }

        totalSpace = GiB(s.f_blocks * s.f_bsize);
        freeSpace = GiB(s.f_bavail * s.f_bsize);
        usedSpace = totalSpace - freeSpace;

        auto formatSize = [](double size) -> std::string
        {
            std::string unit = "GB";
            double displaySize = size;

            if (size >= 1024.0)
            {
                unit = "TB";
                displaySize = size / 1024.0;
            }
            else if (size < 1.0)
            {
                unit = "MB";
                displaySize = size * 1024.0;
            }

            int integerPart = static_cast<int>(displaySize);
            int fractionalPartRounded =
                static_cast<int>((displaySize - integerPart) * 100.0 + 0.5);

            return std::to_string(integerPart) + "." +
                   (fractionalPartRounded < 10 ? "0" : "") +
                   std::to_string(fractionalPartRounded) + " " + unit;
        };

        switch (type)
        {
        case disk_info::PERCENT_USED:
            return std::to_string(percentUsed) + "%";
        case disk_info::TOTAL_SPACE:
            return formatSize(totalSpace);
        case disk_info::USED_SPACE:
            return formatSize(usedSpace);
        case disk_info::FREE_SPACE:
            return formatSize(freeSpace);
        }

        return "";
    }

    std::string get_uptime()
    {
        struct timespec ts;
        int _MONOTONIC = 4;

        int kernel_lib = sceKernelLoadStartModule("/system/common/lib/libkernel.sprx",
                                                  0, nullptr, 0, nullptr, nullptr);

        typedef int32_t (*sceKernelClockGettime_t)(int clock_id, struct timespec *ts);

        sceKernelClockGettime_t _sceKernelClockGettime = nullptr;
        sceKernelDlsym(kernel_lib, "sceKernelClockGettime", (void **)&_sceKernelClockGettime);

        _sceKernelClockGettime(_MONOTONIC, &ts);
        uint64_t total = ts.tv_sec;
        uint64_t d = total / (24 * 3600);
        uint64_t h = (total % (24 * 3600)) / 3600;
        uint64_t m = (total % 3600) / 60;
        uint64_t s = total % 60;

        char buffer[64];
        std::sprintf(buffer, "%luD %luH %luM %luS", d, h, m, s);

        return std::string(buffer);
    }

    std::string get_model()
    {
        std::string model(32, '\0');
        size_t len = model.size();
        sysctlbyname("machdep.icc.hw_model", &model[0], &len, nullptr, 0);
        auto pos = model.find(' ');
        model.resize(pos == std::string::npos ? len : pos);
        return model;
    }

    uint32_t get_cpu_temperature()
    {
        uint32_t celsius;
        sceKernelGetCpuTemperature(&celsius);
        return celsius;
    }

    uint32_t get_soc_temperature()
    {
        uint32_t celsius;
        sceKernelGetSocSensorTemperature(0, &celsius);
        return celsius;
    }

    void set_temperature_limit(uint8_t limit)
    {
        if (limit < 50)
            limit = 50;

        if (limit > 85)
            limit = 85;

        int fd = open("/dev/icc_fan", O_RDONLY, 0);
        if (fd < 0)
            return;

        char data[10] = {0x00, 0x00, 0x00, 0x00, 0x00,
                         static_cast<char>(limit),
                         0x00, 0x00, 0x00, 0x00};

        ioctl(fd, 0xC01C8F07, data);
        close(fd);
    }

    void set_power_state(power_state state)
    {
        int systemState;

        switch (state)
        {
        case power_state::SUSPEND:
            systemState = 0x8004000;
            sceKernelIccIndicatorStandby();
            break;

        case power_state::SHUTDOWN:
            systemState = 0x4000;
            sceKernelIccIndicatorShutdown();
            break;

        case power_state::RESTART:
            systemState = 0x0;
            sceKernelIccIndicatorStandbyShutdown();
            break;

        default:
            return;
        }

        OrbisKernelEventFlag eventFlag;
        sceKernelOpenEventFlag(&eventFlag, "SceSysCoreReboot");
        sceKernelCancelEventFlag(eventFlag, systemState, 0);
        sceKernelCloseEventFlag(eventFlag);
        kill(1, 30);
    }

    void ring_buzzer(int type)
    {
        sceKernelIccSetBuzzer(type);
    }

    std::string get_username()
    {
        int32_t userId;
        sceUserServiceGetForegroundUser(&userId);

        static char username[ORBIS_USER_SERVICE_MAX_USER_NAME_LENGTH + 1];
        if (sceUserServiceGetUserName(userId, username, sizeof(username)) == 0)
            return username;

        return "USER";
    }

    void text_notify(int type, std::string_view msg)
    {
        sceSysUtilSendSystemNotificationWithText(type, msg.data());
    }

    void image_notify(std::string &iconUri, std::string_view text)
    {
        if (iconUri.empty())
            iconUri = "cxml://psnotification/tex_icon_system";

        OrbisNotificationRequest Buffer = {};
        Buffer.type = NotificationRequest;
        Buffer.unk3 = 0;
        Buffer.useIconImageUri = 1;
        Buffer.targetId = -1;

        strncpy(Buffer.message, text.data(), sizeof(Buffer.message) - 1);
        Buffer.message[sizeof(Buffer.message) - 1] = '\0';

        strncpy(Buffer.iconUri, iconUri.c_str(), sizeof(Buffer.iconUri) - 1);
        Buffer.iconUri[sizeof(Buffer.iconUri) - 1] = '\0';

        sceKernelSendNotificationRequest(0, &Buffer, sizeof(Buffer), 0);
    }
}

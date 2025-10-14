#pragma once

#include <orbis/libkernel.h>
#include <orbis/Sysmodule.h>
#include <orbis/SystemService.h>
#include <orbis/UserService.h>
#include <orbis/AppInstUtil.h>
#include <orbis/Bgft.h>
#include <orbis/Net.h>
#include <orbis/Http.h>
#include <orbis/Pad.h>

#include <cstdint>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <fstream>
#include <regex>
#include <thread>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <string_view>

#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/vfs.h>
#include <sys/select.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/user.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include "nlohmann_json.hpp"

#include "globals.hpp"
#include "sfo_parser.hpp"
#include "ps4debug.hpp"

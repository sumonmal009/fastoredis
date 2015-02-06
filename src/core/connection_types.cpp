#include "core/connection_types.h"

namespace
{
    std::vector<std::string> connnectionType = { "Unknown", "Redis", "Memcached" };
    std::vector<std::string> connnectionMode = { "Latency mode", "Slave mode", "Get RDB mode", "Pipe mode",  "Find big keys mode", "Stat mode", "Scan mode", "Intaractive mode" };
}

namespace fastoredis
{
    std::vector<std::string> supportedConnectionTypes()
    {
        return connnectionType;
    }

    std::vector<std::string> supportedConnectionMode()
    {
        return connnectionMode;
    }
}

namespace common
{
    template<>
    fastoredis::connectionTypes convertFromString(const std::string& text)
    {
        for (uint32_t i = 0; i < connnectionType.size(); ++i){
            if (text == connnectionType[i]){
                return static_cast<fastoredis::connectionTypes>(i);
            }
        }

        return fastoredis::DBUNKNOWN;
    }

    std::string convertToString(fastoredis::connectionTypes t)
    {
        return connnectionType[t];
    }

    std::string convertToString(fastoredis::ConnectionMode t)
    {
        return connnectionMode[t];
    }
}

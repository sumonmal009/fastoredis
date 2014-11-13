#pragma once

/**/

#include <vector>

#include "common/types.h"
#include "global/global.h"
#include "core/connection_types.h"

namespace fastoredis
{
    struct ServerInfo
    {
        connectionTypes type() const;
        virtual common::Value* valueByIndexes(unsigned char property, unsigned char field) const = 0;
        virtual ~ServerInfo();

    protected:
        ServerInfo(connectionTypes type);
        ServerInfo(const ServerInfo& that);
        ServerInfo& operator=(const ServerInfo& that);

    private:
        const connectionTypes type_;
    };

    typedef boost::shared_ptr<ServerInfo> ServerInfoSPtr;

    typedef std::pair<std::string, std::string> PropertyType;

    struct ServerPropertyInfo
    {
        ServerPropertyInfo();
        std::vector<PropertyType> propertyes_;
    };

    ServerPropertyInfo makeServerProperty(const FastoObjectPtr& root);

    struct DataBaseInfo
    {
        DataBaseInfo(const common::string16& name, size_t size);
        common::string16 name_;
        size_t size_;
    };

    inline bool operator == (const DataBaseInfo &lhs, const DataBaseInfo &rhs)
    {
        return lhs.name_ == rhs.name_ && lhs.size_ == rhs.size_;
    }
}
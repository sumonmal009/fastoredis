#pragma once

#include <boost/shared_ptr.hpp>

#include "core/connection_types.h"
#include "core/redis/redis_config.h"
#include "core/ssh_info.h"

namespace fastoredis
{
    class IConnectionSettingsBase
    {
    public:
        virtual ~IConnectionSettingsBase();
        std::string hash() const;

        std::string loggingPath() const;

        virtual std::string commandLine() const = 0;
        virtual void setCommandLine(const std::string& line) = 0;

        virtual std::string fullAddress() const = 0;

        virtual std::string host() const = 0;
        virtual int port() const = 0;
        virtual void setPort(int port) = 0;

        std::string connectionName() const;
        void setConnectionName(const std::string &name);

        virtual connectionTypes connectionType() const;
        static IConnectionSettingsBase *fromString(const std::string& val);
        std::string toString() const;

        virtual IConnectionSettingsBase* clone () const = 0;

        bool loggingEnabled() const;
        void setLoggingEnabled(bool isLogging);

        int loggingMsTimeInterval() const;

        SSHInfo sshInfo() const;
        void setSshInfo(const SSHInfo& info);

    protected:
        virtual std::string toCommandLine() const = 0;
        virtual void initFromCommandLine(const std::string& val) = 0;
        IConnectionSettingsBase(const std::string& connectionName);

    private:
        std::string connectionName_;
        std::string hash_;
        bool logging_enabled_;
        SSHInfo sshInfo_;
    };

    const char *useHelpText(connectionTypes type);

    typedef boost::shared_ptr<IConnectionSettingsBase> IConnectionSettingsBaseSPtr;

    class RedisConnectionSettings
            : public IConnectionSettingsBase
    {
    public:
        RedisConnectionSettings(const std::string& connectionName, const redisConfig& info);

        virtual std::string commandLine() const;
        virtual void setCommandLine(const std::string& line);

        virtual std::string fullAddress() const;

        virtual std::string host() const;
        virtual int port() const;
        virtual void setPort(int port);

        virtual connectionTypes connectionType() const;

        redisConfig info() const;
        void setInfo(const redisConfig& info);

        virtual IConnectionSettingsBase* clone() const;

    private:
        virtual std::string toCommandLine() const;
        virtual void initFromCommandLine(const std::string& val);
        redisConfig info_;
    };
}

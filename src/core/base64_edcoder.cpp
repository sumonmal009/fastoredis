#include "core/base64_edcoder.h"

#include "common/utils.h"

namespace fastoredis
{
    Base64EDcoder::Base64EDcoder()
        : IEDcoder(Base64)
    {

    }

    common::ErrorValueSPtr Base64EDcoder::encodeImpl(const std::string& data, std::string& out)
    {
        out = common::utils::base64::encode64(data);
        return common::ErrorValueSPtr();
    }

    common::ErrorValueSPtr Base64EDcoder::decodeImpl(const std::string& data, std::string& out)
    {
        out = common::utils::base64::decode64(data);
        return common::ErrorValueSPtr();
    }
}
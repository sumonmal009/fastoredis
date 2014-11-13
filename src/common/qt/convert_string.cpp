#include "common/qt/convert_string.h"

namespace common
{
    string16 convertToString16(const QString &value)
    {
        return string16((const char16*)value.constData(), sizeof(QChar) * value.size());
    }

    template<>
    QString convertFromString16(const string16& value)
    {
#if defined(WCHAR_T_IS_UTF16)
        const QChar* unicode = (const QChar *)value.c_str();
        return QString(unicode, value.size());
#elif defined(WCHAR_T_IS_UTF32)
        return QString::fromUtf8((const char*)value.c_str(), value.size()*2);
#endif
    }

    std::string convertToString(const QString& from)
    {
        QByteArray sUtf8 = from.toUtf8();
        return std::string(sUtf8.constData(), sUtf8.length());
    }

    template<>
    QString convertFromString<QString>(const std::string& value)
    {
        return QString::fromUtf8((const char*)value.c_str(), value.size());
    }
}
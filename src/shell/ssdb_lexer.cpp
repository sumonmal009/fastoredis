#include "shell/ssdb_lexer.h"

#include "core/ssdb/ssdb_driver.h"

namespace
{
    const QString help("help");
}

namespace fastoredis
{
    SsdbApi::SsdbApi(QsciLexer *lexer)
        : QsciAbstractAPIs(lexer)
    {
    }

    void SsdbApi::updateAutoCompletionList(const QStringList& context, QStringList& list)
    {
        for(QStringList::const_iterator it = context.begin(); it != context.end(); ++it){
            QString val = *it;
            for(std::vector<QString>::const_iterator jt = ssdbCommandsKeywords.begin(); jt != ssdbCommandsKeywords.end(); ++jt){
                QString jval = *jt;
                if(jval.startsWith(val, Qt::CaseInsensitive) || (val == ALL_COMMANDS && context.size() == 1) ){
                    list.append(jval + "?1");
                }
            }

            if(help.startsWith(val, Qt::CaseInsensitive) || (val == ALL_COMMANDS && context.size() == 1) ){
                list.append(help + "?2");
            }
        }
    }

    QStringList SsdbApi::callTips(const QStringList& context, int commas, QsciScintilla::CallTipsStyle style, QList<int>& shifts)
    {
        return QStringList();
    }

    SsdbLexer::SsdbLexer(QObject* parent)
        : QsciLexerCustom(parent)
    {
        setAPIs(new SsdbApi(this));
    }

    const char *SsdbLexer::language() const
    {
        return "Ssdb";
    }

    const char* SsdbLexer::version()
    {
        return SsdbDriver::versionApi();
    }

    QString SsdbLexer::description(int style) const
    {
        switch (style)
        {
        case Default:
             return "Default";
        case Command:
            return "Command";
        case HelpKeyword:
            return "HelpKeyword";
        }

        return QString(style);
    }

    QColor SsdbLexer::defaultColor(int style) const
    {
        switch(style) {
            case Default:
                return Qt::black;
            case Command:
                return Qt::red;
            case HelpKeyword:
                return Qt::red;
        }

        return Qt::black;
    }

    void SsdbLexer::styleText(int start, int end)
    {
        if(!editor()){
            return;
        }

        char *data = new char[end - start + 1];
        editor()->SendScintilla(QsciScintilla::SCI_GETTEXTRANGE, start, end, data);
        QString source(data);
        delete [] data;

        if(source.isEmpty()){
            return;
        }

        paintCommands(source, start);

        int index = 0;
        int begin = 0;
        while( (begin = source.indexOf(help, index, Qt::CaseInsensitive)) != -1){
            index = begin + help.length();

            startStyling(start + begin);
            setStyling(help.length(), HelpKeyword);
            startStyling(start + begin);
        }
    }

    void SsdbLexer::paintCommands(const QString& source, int start)
    {
        for(std::vector<QString>::const_iterator it = ssdbCommandsKeywords.begin(); it != ssdbCommandsKeywords.end(); ++it){
            QString word = (*it);
            int index = 0;
            int begin = 0;
            while( (begin = source.indexOf(word, index, Qt::CaseInsensitive)) != -1){
                index = begin + word.length();

                startStyling(start + begin);
                setStyling(word.length(), Command);
                startStyling(start + begin);
            }
        }
    }
}
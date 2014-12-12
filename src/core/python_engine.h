#pragma once

/**/
#ifdef PYTHON_ENABLED
#include "PythonQtObjectPtr.h"
#endif

#include <QObject>
#include <QString>

#include "global/global.h"
#include "common/patterns/singleton_pattern.h"

namespace fastoredis
{
    bool isPythonEnabled();

    class PythonWorker
            : public QObject
    {
        Q_OBJECT
    public:
        friend class PythonEngine;

        void execute(const QString& script, const QStringList& args);
        void executeScript(const QString& path, const QStringList& args);
        void stop();

        ~PythonWorker();

    Q_SIGNALS:
        //! emitted when python outputs something to stdout (and redirection is turned on)
        void pythonStdOut(const QString& str);
        //! emitted when python outputs something to stderr (and redirection is turned on)
        void pythonStdErr(const QString& str);
        //! emitted when both custom SystemExit exception handler is enabled and a SystemExit
        //! exception is raised.
        //! \sa setSystemExitExceptionHandlerEnabled(bool)
        void systemExitExceptionRaised(int exitCode);
        void executeProgress(int val);

    private Q_SLOTS:
        void init();

    protected:
        virtual void customEvent(QEvent *event);

    private:
        void executeImpl(const std::string& script, const std::vector<std::string> &args);
        void executeScriptImpl(const std::string& path, const std::vector<std::string> &args);

        bool handleError();
        PythonWorker();
        volatile bool stop_;
    #ifdef PYTHON_ENABLED
        PythonQtObjectPtr sys_;
        PythonQtObjectPtr os_;
    #endif
    };

    class PythonEngine
            : public common::patterns::lazy_singleton<PythonEngine>
    {
        friend class common::patterns::lazy_singleton<PythonEngine>;

    public:
        static const char* version();
        std::string execPath() const;
        PythonWorker* createWorker() WARN_UNUSED_RESULT;

    private:
        PythonEngine();
        ~PythonEngine();
    };
}
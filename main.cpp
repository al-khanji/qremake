#include <QtCore>
#include "qscheme.h"

static QSchemeValue print(const QSchemeValue &invocation)
{
    using namespace QtSchemeFunctions;

    for (const auto &arg : invocation.toList())
        qDebug() << arg;

    return QSchemeSymbolLiteral("#t");
}

static QSchemeValue string_split(const QSchemeValue &invocation)
{
    using namespace QtSchemeFunctions;

    const QSchemeValue string = car(invocation);
    if (!is_string(string))
        throw QSchemeException("Expected string argument as first parameter for string_split");

    const QSchemeValue needle = cadr(invocation);
    if (!is_string(needle))
        throw QSchemeException("Expected string argument as second parameter for string_split");

    QString::SplitBehavior behavior = QString::KeepEmptyParts;

    try {
        const QString sym = caddr(invocation).toSymbol().toString();
        behavior = (sym == QStringLiteral("SkipEmptyParts")) ? QString::SkipEmptyParts
                                                             : QString::KeepEmptyParts;
    } catch (QSchemeException) {}

    const QStringList parts = string.toString().split(needle.toString(), behavior);

    QSchemeValueList result;
    for (const QString &s : parts)
        result << s;

    return result;
}

static QSchemeValue exec_system(const QSchemeValue &invocation)
{
    using namespace QtSchemeFunctions;

    QStringList parts;

    for (const auto &val : invocation.toList()) {
        if (is_string(val))
            parts << val.toString();
        else
            throw QSchemeException("Expected string argument for exec_system");
    }

    QProcess process;
    process.setProgram(parts.takeFirst());
    process.setArguments(parts);

    process.start();
    process.waitForFinished(-1);

    const QByteArray out = process.readAllStandardOutput();
    const QByteArray err = process.readAllStandardError();

    return list(QSchemeValue(process.exitCode()),
                QString::fromLocal8Bit(out),
                QString::fromLocal8Bit(err));
}

int main(int argc, char **argv)
{
    QCoreApplication application(argc, argv);

    QSchemeEnvironment environment;

    environment.set(QSchemeSymbolLiteral("system-exec"), exec_system);
    environment.set(QSchemeSymbolLiteral("string-split"), string_split);
    environment.set(QSchemeSymbolLiteral("print"), print);

    environment.load(QStringLiteral(":/system.scm"));
    environment.load(QStringLiteral(":/tests.scm"));

    return 0;
}

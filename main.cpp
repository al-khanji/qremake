#include <QtCore>
#include "qscheme.h"

int main(int argc, char **argv)
{
    QCoreApplication application(argc, argv);

    QSchemeEnvironment environment;
    environment.load(QStringLiteral(":/system.scm"));
    environment.load(QStringLiteral(":/tests.scm"));

    return 0;
}

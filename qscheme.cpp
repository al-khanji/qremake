#include <QtCore/private/qobject_p.h>
#include "qscheme.h"

QT_BEGIN_NAMESPACE

namespace QtSchemeFunctions {

QSchemeValue analyze_define(const QSchemeValue &val)
{
    if (is_list(car(val))) {
        QSchemeValue target = car(val);
        QSchemeValue definition = cadr(val);

        return list(car(target),
                    list(QSchemeSymbolLiteral("lambda"), cdr(target), definition));
    }

    return val;
}

bool is_false(const QSchemeValue &val)
{
    return is_list(val) && val.toList().isEmpty();
}

bool is_symbol(const QSchemeValue &val)
{
    return val.type() == QSchemeValue::Type::Symbol;
}

bool is_number(const QSchemeValue &val)
{
    return val.type() == QSchemeValue::Type::Number;
}

bool is_string(const QSchemeValue &val)
{
    return val.type() == QSchemeValue::Type::String;
}

bool is_list(const QSchemeValue &val)
{
    return val.type() == QSchemeValue::Type::Cons;
}

bool is_native_procedure(const QSchemeValue &val)
{
    return val.type() == QSchemeValue::Type::LambdaProcedure;
}

bool is_foreign_procedure(const QSchemeValue &val)
{
    return val.type() == QSchemeValue::Type::ForeignProcedure;
}

QSchemeValue car(const QSchemeValue &val)
{
    if (is_list(val) && val.toList().size() > 0)
        return val.toList().first();
    else
        throw QSchemeException("car: invalid argument type");
}

QSchemeValue cdr(const QSchemeValue &val)
{
    if (is_list(val) && val.toList().size() > 0)
        return val.toList().mid(1);
    else
        throw QSchemeException("cdr: invalid argument type");
}

QSchemeValue cons(const QSchemeValue &a, const QSchemeValue &b)
{
    if (is_null(b))
        return list(a);

    if (is_list(b)) {
        QSchemeValueList result = b.toList();
        result.prepend(a);
        return result;
    }

    return list(a, b);
}

bool is_null(const QSchemeValue &val)
{
    return is_list(val) && val.toList().isEmpty();
}

} // namespace QtSchemeFunctions

QSchemeValue::QSchemeValue()
    : d(QVariant::fromValue(QSchemeValueList()))
{}

QSchemeValue::QSchemeValue(const QSchemeValue &other)
    : d(QVariant(other.d))
{}

QSchemeValue::~QSchemeValue()
{}

QSchemeValue::QSchemeValue(const QSchemeEnvironment &env)
    : d(QVariant::fromValue(env))
{}

QSchemeValue::QSchemeValue(const QSchemeSymbol &symbol)
    : d(QVariant::fromValue(symbol))
{}

QSchemeValue::QSchemeValue(const QString &string)
    : d(string)
{}

QSchemeValue::QSchemeValue(int i)
    : d(i)
{}

QSchemeValue::QSchemeValue(double d)
    : d(d)
{}

QSchemeValue::QSchemeValue(const QSchemeValueList &list)
    : d(QVariant::fromValue(list))
{}

QSchemeValue::QSchemeValue(foreign_syntax_t syntax)
    : d(QVariant::fromValue(syntax))
{}

QSchemeValue::QSchemeValue(foreign_proc_t proc)
    : d(QVariant::fromValue(proc))
{}

QSchemeValue::QSchemeValue(const QSchemeLambdaProcedure &proc_info)
    : d(QVariant::fromValue(proc_info))
{}

QSchemeValue &QSchemeValue::operator=(const QSchemeValue &other)
{
    d = other.d;
    return *this;
}

bool QSchemeValue::operator==(const QSchemeValue &other) const
{
    return d == other.d;
}

QSchemeValue::Type QSchemeValue::type() const
{
    const int id = d.userType();

    switch (id) {
    case QVariant::String:
        return QSchemeValue::Type::String;

    case QVariant::Int:
    case QVariant::UInt:
    case QVariant::LongLong:
    case QVariant::ULongLong:
    case QVariant::Double:
        return QSchemeValue::Type::Number;

    default:
        break;
    }

    if (id == qMetaTypeId<QSchemeValueList>())
        return QSchemeValue::Type::Cons;
    else if (id == qMetaTypeId<QSchemeSymbol>())
        return QSchemeValue::Type::Symbol;
    else if (id == qMetaTypeId<QSchemeValue::foreign_proc_t>())
        return QSchemeValue::Type::ForeignProcedure;
    else if (id == qMetaTypeId<QSchemeEnvironment>())
        return QSchemeValue::Type::Environment;
    else if (id == qMetaTypeId<QSchemeLambdaProcedure>())
        return QSchemeValue::Type::LambdaProcedure;
    else if (id == qMetaTypeId<QSchemeValue::foreign_syntax_t>())
        return QSchemeValue::Type::ForeignSyntax;

    Q_UNREACHABLE();
}

#define CHECK_TYPE(t) if (type() != t) throw QSchemeException("Invalid type!")

QSchemeEnvironment QSchemeValue::toEnvironment() const
{
    CHECK_TYPE(Type::Environment);
    return d.value<QSchemeEnvironment>();
}

QSchemeSymbol QSchemeValue::toSymbol() const
{
    CHECK_TYPE(Type::Symbol);
    return d.value<QSchemeSymbol>();
}

QSchemeValueList QSchemeValue::toList() const
{
    CHECK_TYPE(Type::Cons);
    return d.value<QSchemeValueList>();
}

QString QSchemeValue::toString() const
{
    CHECK_TYPE(Type::String);
    return d.toString();
}

QVariant QSchemeValue::toNumber() const
{
    CHECK_TYPE(Type::Number);
    return d;
}

QSchemeValue::foreign_syntax_t QSchemeValue::toForeignSyntax() const
{
    CHECK_TYPE(Type::ForeignSyntax);
    return d.value<foreign_syntax_t>();
}

QSchemeValue::foreign_proc_t QSchemeValue::toForeignProcedure() const
{
    CHECK_TYPE(Type::ForeignProcedure);
    return d.value<foreign_proc_t>();
}

QSchemeLambdaProcedure QSchemeValue::toLambdaProcedure() const
{
    CHECK_TYPE(Type::LambdaProcedure);
    return d.value<QSchemeLambdaProcedure>();
}

#undef CHECK_TYPE

QString QSchemeValue::toPrintableString() const
{
    QString string;

    switch (type()) {
    case QSchemeValue::Type::ForeignSyntax:
        string = QStringLiteral("#<ForeignSyntax>");
        break;

    case QSchemeValue::Type::LambdaProcedure:
        string = QStringLiteral("#<Lambda procedure>");
        break;

    case QSchemeValue::Type::Symbol:
        string = toSymbol().toString();
        break;

    case QSchemeValue::Type::Cons:
    {
        QTextStream stream(&string);
        QStringList parts;
        for (const QSchemeValue &v : toList())
            parts.push_back(v.toPrintableString());

        stream << '(' << parts.join(QLatin1Char(' ')) << ')';
    }
        break;

    case QSchemeValue::Type::String:
        string = toString();
        string.replace(QStringLiteral("\""), QStringLiteral("\\\""));
        string = QLatin1Char('"') + string + QLatin1Char('"');
        break;

    case QSchemeValue::Type::Number:
    {
        QTextStream stream(&string);
        stream << toNumber().toDouble();
    }
        break;

    case QSchemeValue::Type::ForeignProcedure:
    {
        QTextStream stream(&string);
        stream << QStringLiteral("#<Foreign ") << (void *) toForeignProcedure() << QStringLiteral(">");
    }
        break;

    case QSchemeValue::Type::Environment:
        string = QStringLiteral("#<Environment>");
        break;
    }

    return string;
}

class QSchemeEnvironmentPrivate : public QEnableSharedFromThis<QSchemeEnvironmentPrivate>
{
public:
    QSharedPointer<QSchemeEnvironmentPrivate> outer;
    QHash<QSchemeSymbol, QSchemeValue> symtab;
};

using namespace QtSchemeFunctions;

static QSchemeValue make_bool(bool b)
{
    return b ? QSchemeSymbolLiteral("#t") : QSchemeSymbolLiteral("#f");
}

static QSchemeValue builtin_define(QSchemeEnvironment &env, const QSchemeValue &arguments)
{
    QSchemeValue simplified = analyze_define(arguments);
    return env.set(car(simplified), env.eval(cadr(simplified)));
}

static QSchemeValue builtin_if(QSchemeEnvironment &env, const QSchemeValue &arguments)
{
    const QSchemeValue predicate = env.eval(car(arguments));
    const QSchemeValue true_branch = cadr(arguments);
    const QSchemeValue false_branch = caddr(arguments);

    return is_true(env.eval(predicate)) ? env.eval(true_branch)
                                        : env.eval(false_branch);
}

static QSchemeValue builtin_eval(QSchemeEnvironment &env, const QSchemeValue &arguments)
{
    return env.eval(arguments);
}

static QSchemeValue builtin_quote(QSchemeEnvironment &, const QSchemeValue &arguments)
{
    return car(arguments);
}

static QSchemeValue builtin_cons(const QSchemeValue &arguments)
{
    return cons(car(arguments), cadr(arguments));
}

static QSchemeValue builtin_car(const QSchemeValue &arguments)
{
    return car(car(arguments));
}

static QSchemeValue builtin_cdr(const QSchemeValue &arguments)
{
    return cdr(car(arguments));
}

static QSchemeValue builtin_lambda(QSchemeEnvironment &env, const QSchemeValue &arguments)
{
    QSchemeLambdaProcedure proc = { car(arguments).toList(), cadr(arguments), env };
    return proc;
}

static QSchemeValue builtin_list(const QSchemeValue &arguments)
{
    return arguments;
}

static QSchemeValue builtin_eqp(const QSchemeValue &arguments)
{
    return make_bool(car(arguments) == cadr(arguments));
}

static QSchemeValue builtin_listp(const QSchemeValue &arguments)
{
    return make_bool(is_list(car(arguments)));
}

static QSchemeValue builtin_stringp(const QSchemeValue &arguments)
{
    return make_bool(is_string(car(arguments)));
}

static QSchemeValue builtin_numberp(const QSchemeValue &arguments)
{
    return make_bool(is_number(car(arguments)));
}

static QSchemeValue builtin_symbolp(const QSchemeValue &arguments)
{
    return make_bool(is_symbol(car(arguments)));
}

static QSchemeValue builtin_callablep(const QSchemeValue &arguments)
{
    const QSchemeValue proc = car(arguments);
    return make_bool(is_foreign_procedure(proc) || is_native_procedure(proc));
}

static QSchemeValue builtin_apply(QSchemeEnvironment &env, const QSchemeValue &arguments)
{
    const QSchemeValue params = env.evalArgumentList(arguments);
    return env.apply(car(params), cadr(params));
}

static const struct {
    const char *name;
    QSchemeValue::foreign_syntax_t proc;
} builtin_syntax[] = {
    { "define", builtin_define },
    { "if", builtin_if },
    { "eval", builtin_eval },
    { "lambda", builtin_lambda },
    { "apply", builtin_apply },
    { "quote", builtin_quote }
};

static const struct {
    const char *name;
    QSchemeValue::foreign_proc_t proc;
} builtin_procedures[] = {
    { "cons", builtin_cons },
    { "car", builtin_car },
    { "cdr", builtin_cdr },
    { "list", builtin_list},
    { "eq?", builtin_eqp },
    { "list?", builtin_listp },
    { "string?", builtin_stringp },
    { "number?", builtin_numberp },
    { "symbol?", builtin_symbolp },
    { "callable?", builtin_callablep },
};

QSchemeEnvironment::QSchemeEnvironment()
    : d_ptr(new QSchemeEnvironmentPrivate)
{
    using namespace QtSchemeFunctions;

    for (const auto &builtin : builtin_syntax)
        set(QSchemeSymbol(QLatin1String(builtin.name)), QSchemeValue(builtin.proc));

    for (const auto &builtin : builtin_procedures)
        set(QSchemeSymbol(QLatin1String(builtin.name)), QSchemeValue(builtin.proc));

    set(QSchemeSymbolLiteral("nil"), list());
    set(QSchemeSymbolLiteral("#f"), list());
    set(QSchemeSymbolLiteral("#t"), QSchemeSymbolLiteral("#t"));
}

QSchemeEnvironment::QSchemeEnvironment(const QSchemeEnvironment &other)
    : d_ptr(other.d_ptr)
{}

QSchemeEnvironment::~QSchemeEnvironment()
{}

QSchemeEnvironment &QSchemeEnvironment::operator=(const QSchemeEnvironment &other)
{
    d_ptr = other.d_ptr;
    return *this;
}

bool QSchemeEnvironment::load(const QString &localPath)
{
    QFile file(localPath);

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open" << localPath << "-" << file.errorString();
        return false;
    }

    QTextStream stream(&file);
    QStringList tokens = tokenize(stream.readAll());

    while (!tokens.isEmpty()) {
        const QSchemeValue exp = readFromTokens(tokens);
        sendToRepl(Message::InputExpression, exp);
        sendToRepl(Message::ResultOfExpression, eval(exp));
    }

    return true;
}

void QSchemeEnvironment::sendToRepl(Message m, const QSchemeValue &val)
{
    switch (m) {
    case Message::InputExpression:
        qDebug() << val;
        break;
    case Message::ResultOfExpression:
        qDebug() << " =>" << val << "\n";
        break;
    }
}

QSchemeEnvironmentPrivate *QSchemeEnvironment::findSymbol(const QSchemeValue &symbol) const
{
    const QSchemeSymbol symname = symbol.toSymbol();
    const QSchemeEnvironmentPrivate *d = d_func();

    while (d && !d->symtab.contains(symname))
        d = d->outer.data();

    return const_cast<QSchemeEnvironmentPrivate *>(d);
}

QSchemeValue QSchemeEnvironment::set(const QSchemeValue &symbol, const QSchemeValue &value)
{
    d_func()->symtab[symbol.toSymbol()] = value;
    return value;
}

QSchemeValue QSchemeEnvironment::get(const QSchemeValue &symbol) const
{
    QSchemeEnvironmentPrivate *envd = findSymbol(symbol);

    if (Q_UNLIKELY(!envd))
        throw QSchemeUndefinedSymbolException(symbol.toSymbol());

    return envd->symtab[symbol.toSymbol()];
}

QSchemeValue QSchemeEnvironment::parse(const QString &program) const
{
    QStringList tokens = tokenize(program);
    return readFromTokens(tokens);
}

namespace Tokens {

static inline bool isDoubleQuote(QChar c) {
    return c == QLatin1Char('"');
}

static inline bool isBackslash(QChar c) {
    return c == QLatin1Char('\\');
}

static inline bool isSpecialSymbol(QChar c) {
    return QStringLiteral("()'").contains(c);
}

static inline bool isNewLine(QChar c) {
    return c == QLatin1Char('\n');
}

static inline bool isSymbolSeparator(QChar c) {
    return c.isSpace() || QStringLiteral("();").contains(c);
}

static inline bool isSemiColon(QChar c) {
    return c == QLatin1Char(';');
}

static inline bool isDoubleQuoted(const QString &token) {
    static const QLatin1Char doubleQuote('"');
    return token.startsWith(doubleQuote) && token.endsWith(doubleQuote) && token.size() > 1;
}

} // namespace Tokens

QStringList QSchemeEnvironment::tokenize(const QString &program) const
{
    using namespace Tokens;

    QStringList tokens;

    enum {
        Normal,
        InString,
        InSymbol,
        InComment
    } state = Normal;

    int sym_beginning = -1;

    for (int index = 0; index < program.size(); index++) {
        const QChar current = program[index];

        switch (state) {
        case Normal:
            if (current.isSpace())
                break;

            if (isSpecialSymbol(current)) {
                tokens << current;
                break;
            }

            if (isSemiColon(current)) {
                state = InComment;
                break;
            }

            state = isDoubleQuote(current) ? InString : InSymbol;
            sym_beginning = index;
            break;

        case InString:
            if (isDoubleQuote(current) && !isBackslash(program[index - 1])) {
                tokens << program.mid(sym_beginning, index - sym_beginning + 1);
                tokens.last().replace(QStringLiteral("\\\""), QStringLiteral("\""));
                state = Normal;
            }
            break;

        case InSymbol:
            if (isSymbolSeparator(current)) {
                tokens << program.mid(sym_beginning, index - sym_beginning);

                if (isSpecialSymbol(current))
                    tokens << current;

                state = Normal;
            }
            break;

        case InComment:
            if (isNewLine(current))
                state = Normal;
            break;
        }
    }

    return tokens;
}

QSchemeValue QSchemeEnvironment::readFromTokens(QStringList &tokens) const
{
    using namespace Tokens;

    if (tokens.isEmpty())
        throw QSchemeException("Unexpected EOF while reading!");

    const QString token = tokens.takeFirst();

    if (token.size() == 1) switch (token[0].toLatin1()) {
    case '(': {
        QSchemeValueList list;

        while (tokens.first() != QStringLiteral(")"))
            list.push_back(readFromTokens(tokens));

        Q_ASSERT(tokens.first() == QStringLiteral(")"));
        tokens.takeFirst();

        return QSchemeValue(list);
    }
        break;

    case ')':
        throw QSchemeException("Unexpected )");
        break;

    case '\'': {
        QSchemeValueList list;

        list.push_back(QSchemeSymbolLiteral("quote"));
        list.push_back(readFromTokens(tokens));

        return QSchemeValue(list);
    }
        break;
    }

    return atomFromToken(token);
}

QSchemeValue QSchemeEnvironment::atomFromToken(const QString &token) const
{
    if (Tokens::isDoubleQuoted(token))
        return QSchemeValue(token.mid(1, token.size() - 2));

    bool ok;

    const int i = token.toInt(&ok);
    if (ok)
        return QSchemeValue(i);

    const double d = token.toDouble(&ok);
    if (ok)
        return QSchemeValue(d);

    return QSchemeValue(QSchemeSymbol(token));
}

QSchemeValue QSchemeEnvironment::eval(const QSchemeValue &exp)
{
    using namespace QtSchemeFunctions;

    QSchemeValue current = exp;

    switch (current.type()) {
    case QSchemeValue::Type::String:
    case QSchemeValue::Type::Number:
    case QSchemeValue::Type::ForeignProcedure:
    case QSchemeValue::Type::ForeignSyntax:
        break;

    case QSchemeValue::Type::Environment:
    case QSchemeValue::Type::LambdaProcedure:
        throw QSchemeException("eval - invalid parameter");
        break;

    case QSchemeValue::Type::Symbol:
        current = get(current);
        break;

    case QSchemeValue::Type::Cons:
    {
        QSchemeValue fn = eval(car(exp));
        QSchemeValue args = cdr(exp);

        if (fn.type() != QSchemeValue::Type::ForeignSyntax)
            args = evalArgumentList(args);

        current = apply(fn, args);
    }
        break;
    }

    return current;

    Q_UNREACHABLE();
}

QSchemeValue QSchemeEnvironment::apply(const QSchemeValue &procedure, const QSchemeValue &arguments)
{
    if (is_foreign_procedure(procedure)) {
        return (procedure.toForeignProcedure())(arguments);
    } else if (procedure.type() == QSchemeValue::Type::ForeignSyntax) {
        return (procedure.toForeignSyntax())(*this, arguments);
    } else if (is_native_procedure(procedure)) {
        return procedure.toLambdaProcedure().apply(arguments);
    } else {
        throw QSchemeException("apply - unknown procedure type");
    }
}

QSchemeValueList QSchemeEnvironment::evalArgumentList(const QSchemeValue &args)
{
    QSchemeValueList result;

    for (const QSchemeValue &arg : args.toList())
        result.push_back(eval(arg));

    return result;
}

QSchemeEnvironment QSchemeEnvironment::makeInner()
{
    QSchemeEnvironment inner;
    inner.d_ptr->outer = this->d_ptr;
    return inner;
}

QSchemeEnvironment::QSchemeEnvironment(QSchemeEnvironmentPrivate *dd)
    : d_ptr(dd->sharedFromThis())
{}

QSchemeValue QSchemeLambdaProcedure::apply(const QSchemeValue &arguments)
{
    const QSchemeValueList arglist = arguments.toList();

    if (Q_UNLIKELY(argnames.size() != arglist.size()))
        throw QSchemeException("Invalid argument count");

    QSchemeEnvironment execution_env = this->environment.makeInner();

    for (int i = 0; i < argnames.size(); i++)
        execution_env.set(argnames[i], arglist[i]);

    return execution_env.eval(this->body);
}

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug d, const QSchemeValue &value)
{
    QDebugStateSaver saver(d);

    d.noquote();
    d << value.toPrintableString();

    return d;
}
#endif

QT_END_NAMESPACE

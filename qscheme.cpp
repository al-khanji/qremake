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

bool is_list(const QSchemeValue &val)
{
    return val.type() == QSchemeValue::Type::Cons;
}


QSchemeValue car(const QSchemeValue &val)
{
    if (val.type() == QSchemeValue::Type::Cons && val.toList().size() > 0)
        return val.toList().first();
    else
        throw QSchemeException("car: invalid argument type");
}

QSchemeValue cdr(const QSchemeValue &val)
{
    if (val.type() != QSchemeValue::Type::Cons)
        throw QSchemeException("cdr: invalid argument type");

    const QSchemeValueList values = val.toList();
    if (values.isEmpty())
        throw QSchemeException("cdr: unexpected empty list");

    return values.mid(1);
}

QSchemeValue cons(const QSchemeValue &val)
{
    QSchemeValue head = car(val);
    QSchemeValue tail = cadr(val);

    if (is_list(tail)) {
        QSchemeValueList result = tail.toList();
        result.prepend(head);
        return result;
    }

    return list(head, tail);
}

} // namespace QtSchemeFunctions

static const quintptr tag_box       = 0b11;
static const quintptr tag_mask      = 0b11;

static inline quintptr tag(quintptr p) { return p & tag_mask; }
static inline quintptr is_box(quintptr p) { return tag(p) == tag_box; }

namespace {
class Box : public QRefcountedData {
public:
    Box() = default;
    Box(QVariant &&v) : variant(v) {}

    QVariant variant;

    static inline quintptr create(QVariant &&v) {
        Box *box = new Box(std::forward<QVariant>(v));
        box->ref.ref();
        return reinterpret_cast<quintptr>(box) | tag_box;
    }

    static inline Box *get(quintptr p) {
        return is_box(p) ? reinterpret_cast<Box *>(p & ~tag_box)
                         : nullptr;
    }

    static inline void attach(quintptr p) {
        if (Box *box = get(p))
            box->ref.ref();
    }

    static inline void detach(quintptr p) {
        if (Box *box = get(p))
            if (!box->ref.deref())
                delete box;
    }

    static inline QSchemeValue::Type type(quintptr p) {
        switch (get(p)->variant.type()) {
        case QVariant::String:
            return QSchemeValue::Type::String;

        case QVariant::Int:
        case QVariant::UInt:
        case QVariant::LongLong:
        case QVariant::ULongLong:
        case QVariant::Double:
            return QSchemeValue::Type::Number;

        case QVariant::UserType:
            break;

        default:
            Q_UNREACHABLE();
        }

        const int id = get(p)->variant.userType();

        if (id == qMetaTypeId<QSchemeValueList>())
            return QSchemeValue::Type::Cons;
        else if (id == qMetaTypeId<QSchemeSymbol>())
            return QSchemeValue::Type::Symbol;
        else if (id == qMetaTypeId<QSchemeValue::foreign_proc_t>())
            return QSchemeValue::Type::ForeignProcedure;
        else if (id == qMetaTypeId<QSchemeEnvironment>())
            return QSchemeValue::Type::Environment;

        Q_UNREACHABLE();
    }
};
} // namespace

QSchemeValue::QSchemeValue()
    : d(Box::create(QVariant::fromValue(QSchemeValueList())))
{}

QSchemeValue::QSchemeValue(const QSchemeValue &other)
    : d(other.d)
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

QSchemeValue::QSchemeValue(foreign_proc_t proc)
    : d(QVariant::fromValue(proc))
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
    switch (d.type()) {
    case QVariant::String:
        return QSchemeValue::Type::String;

    case QVariant::Int:
    case QVariant::UInt:
    case QVariant::LongLong:
    case QVariant::ULongLong:
    case QVariant::Double:
        return QSchemeValue::Type::Number;

    case QVariant::UserType:
        break;

    default:
        Q_UNREACHABLE();
    }

    const int id = d.userType();

    if (id == qMetaTypeId<QSchemeValueList>())
        return QSchemeValue::Type::Cons;
    else if (id == qMetaTypeId<QSchemeSymbol>())
        return QSchemeValue::Type::Symbol;
    else if (id == qMetaTypeId<QSchemeValue::foreign_proc_t>())
        return QSchemeValue::Type::ForeignProcedure;
    else if (id == qMetaTypeId<QSchemeEnvironment>())
        return QSchemeValue::Type::Environment;

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

QSchemeValue::foreign_proc_t QSchemeValue::toForeignProcedure() const
{
    CHECK_TYPE(Type::ForeignProcedure);
    return d.value<foreign_proc_t>();
}

#undef CHECK_TYPE

QString QSchemeValue::toPrintableString() const
{
    QString string;

    switch (type()) {
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

class QSchemeEnvironmentPrivate : public QRefcountedData
{
public:
    QRefcountingPointer<QSchemeEnvironmentPrivate> outer;
    QHash<QSchemeSymbol, QSchemeValue> symtab;
};

enum Builtin_Op {
    Builtin_Define,
    Builtin_If,
    Builtin_Eval,
    Builtin_Quote,
    Builtin_Cons,
    Builtin_Car,
    Builtin_Cdr,
    Builtin_Lambda,
    Builtin_List,
    Builtin_Eq
};

static const QHash<QByteArray, Builtin_Op> builtin_symbol_names = {
    { "define", Builtin_Define },
    { "if", Builtin_If },
    { "eval", Builtin_Eval },
    { "quote", Builtin_Quote },
    { "cons", Builtin_Cons },
    { "car", Builtin_Car },
    { "cdr", Builtin_Cdr },
    { "lambda", Builtin_Lambda },
    { "list", Builtin_List},
    { "eq?", Builtin_Eq }
};

static QSchemeValue dispatch_builtin(QSchemeEnvironment *env, const QSchemeValue &all_args) {
    using namespace QtSchemeFunctions;

    const Builtin_Op op = builtin_symbol_names[car(all_args).toSymbol().toUtf8()];
    const QSchemeValue op_args = cdr(all_args);

    switch (op) {
    case Builtin_Eq:
    {
        const QSchemeValue a = env->eval(car(op_args));
        const QSchemeValue b = env->eval(cadr(op_args));

        return a == b ? QSchemeSymbolLiteral("#t")
                      : QSchemeSymbolLiteral("#f");
    }
        break;

    case Builtin_List:
    {
        QSchemeValueList result;
        for (auto arg : op_args.toList())
            result.push_back(env->eval(arg));
        return result;
    }
        break;

    case Builtin_Define:
    {
        QSchemeValue simplified = analyze_define(op_args);
        return env->set(car(simplified), env->eval(cadr(simplified)));
    }
        break;

    case Builtin_If:
    {
        const QSchemeValue predicate = env->eval(car(op_args));
        const QSchemeValue true_branch = cadr(op_args);
        const QSchemeValue false_branch = caddr(op_args);

        return is_true(env->eval(predicate)) ? env->eval(true_branch)
                                             : env->eval(false_branch);
    }
        break;

    case Builtin_Eval:
        return env->eval(car(op_args));
        break;

    case Builtin_Quote:
        return car(op_args);
        break;

    case Builtin_Cons:
        return cons(list(env->eval(car(op_args)), env->eval(cadr(op_args))));
        break;

    case Builtin_Car:
        return car(env->eval(car(op_args)));
        break;

    case Builtin_Cdr:
        return cdr(env->eval(car(op_args)));
        break;

    case Builtin_Lambda:
        return list(QSchemeSymbolLiteral("procedure"), car(op_args), cadr(op_args), *env);
        break;
    }

    Q_UNREACHABLE();
}

QSchemeEnvironment::QSchemeEnvironment()
    : d_ptr(new QSchemeEnvironmentPrivate)
{
    using namespace QtSchemeFunctions;

    for (const QByteArray &symname : builtin_symbol_names.keys())
        d_ptr->symtab[QSchemeSymbol(symname)] = QSchemeValue(dispatch_builtin);

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
        qWarning() << "Could not open" << localPath;
        return false;
    }

    QTextStream stream(&file);
    while (!stream.atEnd()) {
        const QString line = stream.readLine();
        const QSchemeValue parsed = parse(line);
        const QSchemeValue result = eval(parsed);

        qDebug() << parsed << "\n  =>" << result << "\n";
    }

    return true;
}

QSchemeEnvironment QSchemeEnvironment::findSymbol(const QSchemeValue &symbol) const
{
    const QSchemeSymbol symname = symbol.toSymbol();

    const QSchemeEnvironmentPrivate *d = d_func();

    while (d && !d->symtab.contains(symname))
        d = d->outer.data();

    if (!d)
        throw QSchemeUndefinedSymbolException(symname);

    return QSchemeEnvironment(const_cast<QSchemeEnvironmentPrivate *>(d));
}

QSchemeValue QSchemeEnvironment::set(const QSchemeValue &symbol, const QSchemeValue &value)
{
    d_func()->symtab[symbol.toSymbol()] = value;
    return value;
}

QSchemeValue QSchemeEnvironment::get(const QSchemeValue &symbol) const
{
    return findSymbol(symbol).d_func()->symtab[symbol.toSymbol()];
}

QSchemeValue QSchemeEnvironment::parse(const QString &program) const
{
    QStringList tokens = tokenize(program);
    return readFromTokens(tokens);
}

namespace Tokens {

static constexpr inline bool isDoubleQuote(QChar c) {
    return c == QLatin1Char('"');
}

static constexpr inline bool isBackslash(QChar c) {
    return c == QLatin1Char('\\');
}

static constexpr inline bool isSpecialSymbol(QChar c) {
    return c == QLatin1Char('(') || c == QLatin1Char(')') || c == QLatin1Char('\'');
}

static constexpr inline bool isNewLine(QChar c) {
    return c == QLatin1Char('\n');
}

static constexpr inline bool isSymbolSeparator(QChar c) {
    return c.isSpace() || QStringLiteral("()").contains(c);
}

static constexpr inline bool isSemiColon(QChar c) {
    return c == QLatin1Char(';');
}

static constexpr inline bool isDoubleQuoted(const QString &token) {
    constexpr QLatin1Char doubleQuote('"');
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

    switch (exp.type()) {
    case QSchemeValue::Type::String:
    case QSchemeValue::Type::Number:
    case QSchemeValue::Type::Environment:
    {
        QSchemeValue result = exp;
        return result;
    }
        break;

    case QSchemeValue::Type::Symbol:
    {
        QSchemeValue result = get(exp);
        return result;
    }
        break;

    case QSchemeValue::Type::Cons:
    {
        const QSchemeValue fn = eval(car(exp));

        if (is_list(fn) && is_symbol(car(fn)) && car(fn).toSymbol() == QSchemeSymbolLiteral("procedure")) {
            QSchemeValueList argnames = cadr(fn).toList();
            QSchemeValueList argvalues = cdr(exp).toList();

            if (argnames.size() != argvalues.size())
                throw QSchemeException("Invalid argument count");

            for (QSchemeValue &val : argvalues)
                val = eval(val);

            QSchemeEnvironment execution_env = cadddr(fn).toEnvironment().makeInner();

            for (int i = 0; i < argnames.size(); i++)
                execution_env.set(argnames[i], argvalues[i]);

            QSchemeValue procedure_body = caddr(fn);
            return execution_env.eval(procedure_body);
        }

        if (Q_UNLIKELY(fn.type() != QSchemeValue::Type::ForeignProcedure))
            throw QSchemeException(QStringLiteral("eval: expected procedure, got %1").arg(exp.toPrintableString()));

        QSchemeValue result = (*fn.toForeignProcedure())(this, exp);
        return result;

    }
        break;

    case QSchemeValue::Type::ForeignProcedure:
        return (*exp.toForeignProcedure())(this, QSchemeValueList());
        break;
    }

    Q_UNREACHABLE();
}

QSchemeEnvironment QSchemeEnvironment::makeInner()
{
    QSchemeEnvironment inner;
    inner.d_ptr->outer = this->d_ptr;
    return inner;
}

QSchemeEnvironment::QSchemeEnvironment(QSchemeEnvironmentPrivate *dd)
    : d_ptr(dd)
{}

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

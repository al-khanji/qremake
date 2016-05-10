#ifndef QSCHEME_H
#define QSCHEME_H

#include "qtschemeglobal.h"
#include <QtCore>

QT_BEGIN_NAMESPACE

class QSchemeValue;
class QSchemeEnvironment;

typedef QVector<QSchemeValue> QSchemeValueList;

class QRefcountedData
{
public:
    inline QRefcountedData() : ref(0) {}
    inline QRefcountedData(const QRefcountedData &) : ref(0) {}
    inline QRefcountedData &operator=(const QRefcountedData &) { return *this; }

    mutable QAtomicInt ref;
};

template <class Data>
class QRefcountingPointer
{
public:
    typedef Data type;
    typedef Data* pointer;

    inline QRefcountingPointer() : d(nullptr) {}
    inline QRefcountingPointer(const QRefcountingPointer &other) : d(other.d) { ref(data()); }
    inline QRefcountingPointer(pointer p) : d(p) { ref(p); }
    inline ~QRefcountingPointer() { reset(); }

    inline QRefcountingPointer &operator=(const QRefcountingPointer &other) {
        reset(other.data());
        return *this;
    }

    inline void reset(pointer p = nullptr) {
        if (d != p) {
            pointer previous = data();
            ref(p);
            d = p;
            deref(previous);
        }
    }

    inline type &operator*() const { return *data(); }
    inline pointer operator->() const { return data(); }

    inline pointer data() const { return reinterpret_cast<pointer>(d); }
    inline pointer take() const { QRefcountedData *p = nullptr; std::swap(p, d); return reinterpret_cast<pointer>(p); }

    inline explicit operator bool() const { return isNull() ? nullptr : d; }
    inline bool isNull() const { return !d; }

private:
    inline static void ref(pointer p) { if (p) p->ref.ref(); }
    inline static void deref(pointer p) { if (p && !p->ref.deref()) delete p; }

    QRefcountedData *d = nullptr;
};

class Q_SCHEME_EXPORT QSchemeSymbol {
public:
    inline QSchemeSymbol() {}
    inline explicit QSchemeSymbol(const QLatin1String &string) : m_utf8_name(string.data()) {}
    inline explicit QSchemeSymbol(const QString &string) : m_utf8_name(string.toUtf8()) {}
    inline explicit QSchemeSymbol(const QByteArray &name) : m_utf8_name(name) {}
    inline ~QSchemeSymbol() {}

    inline QSchemeSymbol &operator=(const QSchemeSymbol &other) {
        m_utf8_name = other.m_utf8_name;
        return *this;
    }

    inline bool operator==(const QSchemeSymbol &other) const {
        return m_utf8_name == other.toUtf8();
    }

    inline QByteArray toUtf8() const { return m_utf8_name; }
    inline QString toString() const { return QString::fromUtf8(m_utf8_name); }

private:
    QByteArray m_utf8_name;
};

inline uint qHash(const QSchemeSymbol &sym, uint seed) {
    return qHash(sym.toUtf8(), seed);
}

inline QSchemeSymbol QSchemeSymbolLiteral(const char *symname) {
    return QSchemeSymbol(QByteArray(symname));
}

class Q_SCHEME_EXPORT QSchemeException : public std::exception {
public:
    QSchemeException(const char *message)
        : m_msg(QByteArray(message)) {}
    QSchemeException(const QLatin1String &message)
        : m_msg(message.data()) {}
    QSchemeException(const QString &message)
        : m_msg(message.toUtf8()) {}

    const char *what() const noexcept Q_DECL_OVERRIDE { return m_msg.data(); }

private:
    QByteArray m_msg;
};

class Q_SCHEME_EXPORT QSchemeUndefinedSymbolException : public QSchemeException {
public:
    QSchemeUndefinedSymbolException(const QSchemeSymbol &symbol)
        : QSchemeException(QStringLiteral("Undefined symbol: ") + symbol.toString())
        , m_symbol(symbol)
    {}

    QSchemeSymbol symbol() const { return m_symbol; }

private:
    QSchemeSymbol m_symbol;
};

class Q_SCHEME_EXPORT QSchemeValue
{
public:    
    QSchemeValue();
    QSchemeValue(const QSchemeValue &);
    ~QSchemeValue();

    typedef QSchemeValue (*foreign_proc_t)(QSchemeEnvironment *env, const QSchemeValue &arg);

    QSchemeValue(const QSchemeEnvironment &env);
    QSchemeValue(const QSchemeSymbol &symbol);
    QSchemeValue(const QString &string);
    QSchemeValue(const QSchemeValueList &list); // -> Cons
    QSchemeValue(foreign_proc_t proc);

    explicit QSchemeValue(int i);
    explicit QSchemeValue(double d);

    QSchemeValue &operator=(const QSchemeValue &);
    bool operator==(const QSchemeValue &other) const;

    enum class Type {
        Environment,
        Symbol,
        Cons,
        String,
        Number,
        ForeignProcedure
    };

    Type type() const;

    QSchemeEnvironment toEnvironment() const;
    QSchemeSymbol toSymbol() const;
    QSchemeValueList toList() const;
    QString toString() const;
    QVariant toNumber() const;
    foreign_proc_t toForeignProcedure() const;

    QString toPrintableString() const;

private:
    QVariant d;
};

namespace QtSchemeFunctions {
// simplify arguments to define, e.g. ((double n) (* 2 n)) becomes (double (lambda (n) (* 2 n))
Q_SCHEME_EXPORT QSchemeValue analyze_define(const QSchemeValue &val);

Q_SCHEME_EXPORT bool is_false(const QSchemeValue &val);
inline bool is_true(const QSchemeValue &val) { return !is_false(val); }

Q_SCHEME_EXPORT bool is_symbol(const QSchemeValue &val);
Q_SCHEME_EXPORT bool is_list(const QSchemeValue &val);

Q_SCHEME_EXPORT QSchemeValue car(const QSchemeValue &val);
Q_SCHEME_EXPORT QSchemeValue cdr(const QSchemeValue &val);
Q_SCHEME_EXPORT QSchemeValue cons(const QSchemeValue &val);

template <class... Ts>
inline QSchemeValue list(const Ts& ...values) {
    return QSchemeValueList { values... };
}

inline QSchemeValue caar(const QSchemeValue &val) {
    return car(car(val));
}

inline QSchemeValue cadr(const QSchemeValue &val) {
    return car(cdr(val));
}

inline QSchemeValue cdar(const QSchemeValue &val) {
    return cdr(car(val));
}

inline QSchemeValue cddr(const QSchemeValue &val) {
    return cdr(cdr(val));
}

inline QSchemeValue cadar(const QSchemeValue &val) {
    return car(cdr(car(val)));
}

inline QSchemeValue caddr(const QSchemeValue &val) {
    return car(cdr(cdr(val)));
}

inline QSchemeValue cdaar(const QSchemeValue &val) {
    return cdr(car(car(val)));
}

inline QSchemeValue cadaar(const QSchemeValue &val) {
    return car(cdr(car(car(val))));
}

inline QSchemeValue cadddr(const QSchemeValue &val) {
    return car(cdr(cdr(cdr(val))));
}

inline QSchemeValue cddddr(const QSchemeValue &val) {
    return cdr(cdr(cdr(cdr(val))));
}

}

class QSchemeEnvironmentPrivate;
class Q_SCHEME_EXPORT QSchemeEnvironment
{
public:
    QSchemeEnvironment();
    QSchemeEnvironment(const QSchemeEnvironment &);
    virtual ~QSchemeEnvironment();

    QSchemeEnvironment &operator=(const QSchemeEnvironment &);

    bool load(const QString &localPath);

    virtual QSchemeEnvironment findSymbol(const QSchemeValue &symbol) const;

    virtual QSchemeValue set(const QSchemeValue &symbol, const QSchemeValue &value);
    virtual QSchemeValue get(const QSchemeValue &symbol) const;

    virtual QSchemeValue parse(const QString &program) const;
    virtual QStringList tokenize(const QString &program) const;
    virtual QSchemeValue readFromTokens(QStringList &tokens) const;
    virtual QSchemeValue atomFromToken(const QString &token) const;

    virtual QSchemeValue eval(const QSchemeValue &exp);

    virtual QSchemeEnvironment makeInner();

protected:
    QSchemeEnvironment(QSchemeEnvironmentPrivate *dd);

private:
    QRefcountingPointer<QSchemeEnvironmentPrivate> d_ptr;
    Q_DECLARE_PRIVATE(QSchemeEnvironment)
};

#ifndef QT_NO_DATASTREAM
Q_SCHEME_EXPORT QDataStream &operator<<(QDataStream &, const QSchemeValue &);
Q_SCHEME_EXPORT QDataStream &operator>>(QDataStream &, QSchemeValue &);
#endif

#ifndef QT_NO_DEBUG_STREAM
Q_SCHEME_EXPORT QDebug operator<<(QDebug, const QSchemeValue &);
#endif

Q_DECLARE_METATYPE(QSchemeSymbol)
Q_DECLARE_METATYPE(QSchemeValue)
Q_DECLARE_METATYPE(QSchemeEnvironment)
Q_DECLARE_METATYPE(QSchemeValueList)
Q_DECLARE_METATYPE(QSchemeValue::foreign_proc_t)

QT_END_NAMESPACE

#endif // QSCHEME_H

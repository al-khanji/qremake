#include <QtCore/private/qobject_p.h>
#include "qscheme.h"

QT_BEGIN_NAMESPACE

static const quintptr tag_undefined = 0b01;
static const quintptr tag_nil       = 0b10;
static const quintptr tag_box       = 0b11;
static const quintptr tag_mask      = 0b11;

static inline quintptr specialValueToTag(QSchemeValue::SpecialValue v) {
    switch (v) {
    case QSchemeValue::UndefinedValue:
        return tag_undefined;
        break;
    case QSchemeValue::NilValue:
        return tag_nil;
        break;
    }
}

static inline quintptr tag(quintptr p) { return p & tag_mask; }

static inline quintptr is_undefined(quintptr p) { return tag(p) == tag_undefined; }
static inline quintptr is_nil(quintptr p) { return tag(p) == tag_nil; }
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
        return reinterpret_cast<quintptr>(box);
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
        case QVariant::Bool:
            return QSchemeValue::Type::Bool;
        case QVariant::String:
            return QSchemeValue::Type::String;
        case QVariant::Int:
        case QVariant::UInt:
        case QVariant::LongLong:
        case QVariant::ULongLong:
        case QVariant::Double:
            return QSchemeValue::Type::Number;
        default:
            Q_UNREACHABLE();
        }
    }
};
} // namespace

QSchemeValue::QSchemeValue(QSchemeValue::SpecialValue value)
    : d(specialValueToTag(value))
{}

QSchemeValue::QSchemeValue(const QSchemeValue &other)
    : d(other.d)
{
    Box::attach(d);
}

QSchemeValue::~QSchemeValue()
{
    Box::detach(d);
}

QSchemeValue::QSchemeValue(const QString &string)
    : d(Box::create(string))
{}

QSchemeValue::QSchemeValue(bool b)
    : d(Box::create(b))
{}

QSchemeValue::QSchemeValue(int i)
    : d(Box::create(i))
{}

QSchemeValue &QSchemeValue::operator=(const QSchemeValue &other)
{
    if (d != other.d) {
        Box::detach(d);
        d = other.d;
        Box::attach(d);
    }

    return *this;
}

QSchemeValue::Type QSchemeValue::type() const
{
    switch (tag(d)) {
    case tag_undefined:
        return QSchemeValue::Type::Undefined;
    case tag_nil:
        return QSchemeValue::Type::Nil;
    case tag_box:
        return Box::type(d);
    }

    Q_UNREACHABLE();
}

typedef QSchemeValue value_t;
typedef QByteArray symbol_t;

class QSchemeEnvironmentPrivate : public QRefcountedData
{
    QRefcountingPointer<QSchemeEnvironmentPrivate> outer;
    QHash<symbol_t, value_t> symtab;
};

QSchemeEnvironment::QSchemeEnvironment()
    : d_ptr(new QSchemeEnvironmentPrivate)
{}

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

QT_END_NAMESPACE

#ifndef QSCHEME_H
#define QSCHEME_H

#include "qtschemeglobal.h"
#include <QtCore>

QT_BEGIN_NAMESPACE

class QRefcountedData
{
public:
    QRefcountedData() : ref(0) {}
    QRefcountedData(const QRefcountedData &) : ref(0) {}
    QRefcountedData &operator=(const QRefcountedData &) { return *this; }

    mutable QAtomicInt ref;
};

template <class Data>
class QRefcountingPointer
{
public:
    typedef Data type;
    typedef Data* pointer;

    inline QRefcountingPointer() : d(nullptr) {}
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

class QSchemeValue
{
public:
    enum SpecialValue {
        UndefinedValue, NilValue
    };

    QSchemeValue(SpecialValue value = UndefinedValue);
    QSchemeValue(const QSchemeValue &);
    ~QSchemeValue();

    QSchemeValue(const QString &string);
    QSchemeValue(bool b);
    QSchemeValue(int i);

    QSchemeValue &operator=(const QSchemeValue &);

    enum class Type {
        Undefined,
        Nil,
        String,
        Bool,
        Number
    };

    Type type() const;

private:
    quintptr d;
};

class QSchemeEnvironmentPrivate;
class QSchemeEnvironment
{
public:
    QSchemeEnvironment();
    QSchemeEnvironment(const QSchemeEnvironment &);
    virtual ~QSchemeEnvironment();

    QSchemeEnvironment &operator=(const QSchemeEnvironment &);

private:
    QRefcountingPointer<QSchemeEnvironmentPrivate> d_ptr;
    Q_DECLARE_PRIVATE(QSchemeEnvironment)
};

QT_END_NAMESPACE

#endif // QSCHEME_H

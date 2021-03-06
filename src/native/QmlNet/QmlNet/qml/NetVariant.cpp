#include <QmlNet/qml/NetVariant.h>
#include <QmlNet/qml/NetValue.h>
#include <QmlNet/qml/NetJsValue.h>
#include <QmlNetUtilities.h>
#include <QDateTime>
#include <QDebug>
#include <QJSEngine>

struct NetReferenceQmlContainer
{
    QSharedPointer<NetReference> netReference;
};

struct NetJsValueQmlContainer
{
    QSharedPointer<NetJSValue> jsValue;
};

Q_DECLARE_METATYPE(NetReferenceQmlContainer)
Q_DECLARE_METATYPE(NetJsValueQmlContainer)

NetVariant::NetVariant()
{

}

NetVariant::~NetVariant()
{
    clearNetReference();
}

NetVariantTypeEnum NetVariant::getVariantType()
{
    switch(variant.type()) {
    case QVariant::Invalid:
        return NetVariantTypeEnum_Invalid;
    case QVariant::Bool:
        return NetVariantTypeEnum_Bool;
    case QVariant::Char:
        return NetVariantTypeEnum_Char;
    case QVariant::Int:
        return NetVariantTypeEnum_Int;
    case QVariant::UInt:
        return NetVariantTypeEnum_UInt;
    case QVariant::Double:
        return NetVariantTypeEnum_Double;
    case QVariant::String:
        return NetVariantTypeEnum_String;
    case QVariant::DateTime:
        return NetVariantTypeEnum_DateTime;
    case QVariant::UserType:
        if(strcmp(variant.typeName(), "NetReferenceQmlContainer") == 0)
            return NetVariantTypeEnum_Object;
        if(strcmp(variant.typeName(), "NetJsValueQmlContainer") == 0)
            return NetVariantTypeEnum_JSValue;
        qWarning() << "Unknown user type for NetVariant: " << variant.typeName();
        return NetVariantTypeEnum_Invalid;
    default:
        qWarning() << "Unsupported qt variant type: " << variant.type();
        return NetVariantTypeEnum_Invalid;
    }
}

void NetVariant::setNetReference(QSharedPointer<NetReference> netReference)
{
    clearNetReference();
    variant.setValue(NetReferenceQmlContainer{ netReference });
}

QSharedPointer<NetReference> NetVariant::getNetReference()
{
    if(getVariantType() == NetVariantTypeEnum_Object) {
        return variant.value<NetReferenceQmlContainer>().netReference;
    }
    return nullptr;
}

void NetVariant::setBool(bool value)
{
    clearNetReference();
    variant.setValue(value);
}

bool NetVariant::getBool()
{
    if(variant.canConvert(QMetaType::Bool))
        return variant.value<int>();

    qDebug() << "Can't convert value to bool";

    return false;
}

void NetVariant::setChar(QChar value)
{
    clearNetReference();
    variant.setValue(value);
}

QChar NetVariant::getChar()
{
    return variant.toChar();
}

void NetVariant::setInt(int value)
{
    clearNetReference();
    variant.setValue(value);
}

int NetVariant::getInt()
{
    if(variant.canConvert(QMetaType::Int))
        return variant.value<int>();

    qDebug() << "Can't convert value to int";

    return 0;
}

void NetVariant::setUInt(unsigned int value)
{
    clearNetReference();
    variant.setValue(value);
}

unsigned int NetVariant::getUInt()
{
    bool ok = false;
    unsigned int result = variant.toUInt(&ok);

    if(!ok) {
        qDebug() << "Couldn't convert variant to unsigned int";
    }

    return result;
}

void NetVariant::setDouble(double value)
{
    clearNetReference();
    variant.setValue(value);
}

double NetVariant::getDouble()
{
    bool ok = false;
    double result = variant.toDouble(&ok);

    if(!ok) {
        qDebug() << "Couldn't convert variant to double";
    }

    return result;
}

void NetVariant::setString(QString* value)
{
    clearNetReference();
    if(value) {
        variant.setValue(*value);
    } else {
        variant.clear();
    }
}

QString NetVariant::getString()
{
    if(variant.type() != QVariant::String) {
        qDebug() << "Variant is not a string";
        return QString();
    }

    return variant.toString();
}

void NetVariant::setDateTime(QDateTime& value)
{
    clearNetReference();
    if(value.isNull()) {
        variant.clear();
    } else {
        variant.setValue(value);
    }
}

QDateTime NetVariant::getDateTime()
{
    return variant.toDateTime();
}

void NetVariant::setJsValue(QSharedPointer<NetJSValue> jsValue)
{
    clearNetReference();
    variant.setValue(NetJsValueQmlContainer{ jsValue });
}

QSharedPointer<NetJSValue> NetVariant::getJsValue()
{
    if(getVariantType() == NetVariantTypeEnum_JSValue) {
        return variant.value<NetJsValueQmlContainer>().jsValue;
    }
    return nullptr;
}

void NetVariant::clear()
{
    clearNetReference();
    variant.clear();
}

QSharedPointer<NetVariant> NetVariant::fromQJSValue(const QJSValue& qJsValue)
{
    QSharedPointer<NetVariant> result;
    if(qJsValue.isNull() || qJsValue.isUndefined()) {
        // Nothing!
    }
    else if(qJsValue.isQObject()) {
        QObject* qObject = qJsValue.toQObject();
        NetValueInterface* netValue = qobject_cast<NetValueInterface*>(qObject);
        if(!netValue) {
            qWarning() << "Return type must be a JS type/object, or a .NET object.";
        } else {
            result = QSharedPointer<NetVariant>(new NetVariant());
            result->setNetReference(netValue->getNetReference());
        }
    }
    else if(qJsValue.isObject()) {
        result = QSharedPointer<NetVariant>(new NetVariant());
        result->setJsValue(QSharedPointer<NetJSValue>(new NetJSValue(qJsValue)));
    }
    else {
        result = QSharedPointer<NetVariant>(new NetVariant());
        QVariant variant = qJsValue.toVariant();
        result->variant = variant;
    }
    return result;
}

QJSValue NetVariant::toQJSValue(QJSEngine* jsEngine)
{
    switch(getVariantType()) {
    case NetVariantTypeEnum_Object: {
        NetValue* netValue = NetValue::forInstance(getNetReference());
        return jsEngine->newQObject(netValue);
    }
    case NetVariantTypeEnum_JSValue: {
        return getJsValue()->getJsValue();
    }
    default: {
        return jsEngine->toScriptValue<QVariant>(toQVariant());
    }
    }
}

void NetVariant::fromQVariant(const QVariant* variant, QSharedPointer<NetVariant> destination)
{
    switch(variant->type()) {
    case QVariant::Invalid:
        destination->clear();
        break;
    case QVariant::Bool:
        destination->setBool(variant->value<bool>());
        break;
    case QVariant::Char:
        destination->setChar(variant->toChar());
        break;
    case QVariant::Int:
        destination->setInt(variant->value<int>());
        break;
    case QVariant::UInt:
        destination->setUInt(variant->value<unsigned int>());
        break;
    case QVariant::Double:
        destination->setDouble(variant->value<double>());
        break;
    case QVariant::String:
    {
        QString stringValue = variant->toString();
        destination->setString(&stringValue);
        break;
    }
    case QVariant::DateTime:
    {
        QDateTime dateTimeValue = variant->toDateTime();
        destination->setDateTime(dateTimeValue);
        break;
    }
    default:

        if(variant->userType() == qMetaTypeId<QJSValue>()) {
            // TODO: Either serialize this type to a string, to be deserialized in .NET, or
            // pass raw value to .NET to be dynamically invoked (using dynamic).
            // See qtdeclarative\src\plugins\qmltooling\qmldbg_debugger\qqmlenginedebugservice.cpp:184
            // for serialization methods.
            QSharedPointer<NetJSValue> netJsValue = QSharedPointer<NetJSValue>(new NetJSValue(variant->value<QJSValue>()));
            destination->setJsValue(netJsValue);
            break;
        }

        if(variant->userType() == QMetaType::QObjectStar) {
            QObject* value = variant->value<QObject*>();
            NetValueInterface* netValue = qobject_cast<NetValueInterface*>(value);
            if(netValue) {
                destination->setNetReference(netValue->getNetReference());
                break;
            }
        }

        qDebug() << "Unsupported variant type: " << variant->type();
        break;
    }
}

QSharedPointer<NetVariant> NetVariant::fromQVariant(const QVariant* variant)
{
    QSharedPointer<NetVariant> result = QSharedPointer<NetVariant>(new NetVariant());
    fromQVariant(variant, result);
    return result;
}

QVariant NetVariant::toQVariant()
{
    QVariant result;
    switch(getVariantType()) {
    case NetVariantTypeEnum_JSValue:
        result = getJsValue()->getJsValue().toVariant();
        break;
    case NetVariantTypeEnum_Object:
        result = QVariant::fromValue<QObject*>(NetValue::forInstance(getNetReference()));
        break;
    default:
        result = variant;
        break;
    }
    return result;
}

QString NetVariant::getDisplayValue()
{
    QString result;
    switch(getVariantType()) {
    case NetVariantTypeEnum_JSValue:
        result = getJsValue()->getJsValue().toString();
        break;
    case NetVariantTypeEnum_Object:
        result = getNetReference()->displayName();
        break;
    default:
        result = variant.toString();
        break;
    }
    return result;
}

void NetVariant::clearNetReference()
{
    if(variant.canConvert<NetReferenceQmlContainer>())
    {
        variant.value<NetReferenceQmlContainer>().netReference.clear();
        variant.clear();
    }
    else if(variant.canConvert<NetJsValueQmlContainer>())
    {
        variant.value<NetJsValueQmlContainer>().jsValue.clear();
        variant.clear();
    }
}

extern "C" {

struct Q_DECL_EXPORT DateTimeContainer {
    bool isNull;
    int month;
    int day;
    int year;
    int hour;
    int minute;
    int second;
    int msec;
    int offsetSeconds;
};

Q_DECL_EXPORT NetVariantContainer* net_variant_create() {
    NetVariantContainer* result = new NetVariantContainer();
    result->variant = QSharedPointer<NetVariant>(new NetVariant());
    return result;
}

Q_DECL_EXPORT void net_variant_destroy(NetVariantContainer* container) {
    delete container;
}

Q_DECL_EXPORT NetVariantTypeEnum net_variant_getVariantType(NetVariantContainer* container) {
    return container->variant->getVariantType();
}

Q_DECL_EXPORT void net_variant_setNetReference(NetVariantContainer* container, NetReferenceContainer* instanceContainer) {
    if(instanceContainer == nullptr) {
        container->variant->setNetReference(nullptr);
    } else {
        container->variant->setNetReference(instanceContainer->instance);
    }
}

Q_DECL_EXPORT NetReferenceContainer* net_variant_getNetReference(NetVariantContainer* container) {
    QSharedPointer<NetReference> instance = container->variant->getNetReference();
    if(instance == nullptr) {
        return nullptr;
    }
    NetReferenceContainer* result = new NetReferenceContainer();
    result->instance = instance;
    return result;
}

Q_DECL_EXPORT void net_variant_setBool(NetVariantContainer* container, bool value) {
    container->variant->setBool(value);
}

Q_DECL_EXPORT bool net_variant_getBool(NetVariantContainer* container) {
    return container->variant->getBool();
}

Q_DECL_EXPORT void net_variant_setChar(NetVariantContainer* container, ushort value) {
    container->variant->setChar(value);
}

Q_DECL_EXPORT ushort net_variant_getChar(NetVariantContainer* container) {
    return (ushort)container->variant->getChar().unicode();
}

Q_DECL_EXPORT void net_variant_setInt(NetVariantContainer* container, int value) {
    container->variant->setInt(value);
}

Q_DECL_EXPORT int net_variant_getInt(NetVariantContainer* container) {
    return container->variant->getInt();
}

Q_DECL_EXPORT void net_variant_setUInt(NetVariantContainer* container, unsigned int value) {
    container->variant->setUInt(value);
}

Q_DECL_EXPORT unsigned int net_variant_getUInt(NetVariantContainer* container) {
    return container->variant->getUInt();
}

Q_DECL_EXPORT void net_variant_setDouble(NetVariantContainer* container, double value) {
    container->variant->setDouble(value);
}

Q_DECL_EXPORT double net_variant_getDouble(NetVariantContainer* container) {
    return container->variant->getDouble();
}

Q_DECL_EXPORT void net_variant_setString(NetVariantContainer* container, LPWSTR value) {
    if(value == nullptr) {
        container->variant->setString(nullptr);
    } else {
        QString temp = QString::fromUtf16((const char16_t*)value);
        container->variant->setString(&temp);
    }
}

Q_DECL_EXPORT QmlNetStringContainer* net_variant_getString(NetVariantContainer* container) {
    QString string = container->variant->getString();
    if(string.isNull()) {
        return nullptr;
    }
    return createString(string);
}

Q_DECL_EXPORT void net_variant_setDateTime(NetVariantContainer* container, DateTimeContainer* value) {
    if(value == nullptr || value->isNull) {
        QDateTime dt;
        container->variant->setDateTime(dt);
    } else {
        QDateTime dt(QDate(value->year, value->month, value->day),
                     QTime(value->hour, value->minute, value->second, value->msec),
                     Qt::OffsetFromUTC, value->offsetSeconds);
        container->variant->setDateTime(dt);
    }
}
Q_DECL_EXPORT void net_variant_getDateTime(NetVariantContainer* container, DateTimeContainer* value) {
    QDateTime dt = container->variant->getDateTime();
    if(dt.isNull()) {
        value->isNull = true;
        return;
    }
    if(!dt.isValid()) {
        qWarning("QDateTime is invalid");
        value->isNull = true;
        return;
    }
    value->year = dt.date().year();
    value->month = dt.date().month();
    value->day = dt.date().day();
    value->hour = dt.time().hour();
    value->minute = dt.time().minute();
    value->second = dt.time().second();
    value->msec = dt.time().msec();
    value->offsetSeconds = dt.offsetFromUtc();
}

Q_DECL_EXPORT void net_variant_setJsValue(NetVariantContainer* container, NetJSValueContainer* jsValueContainer) {
    if(jsValueContainer == nullptr) {
        container->variant->setJsValue(nullptr);
    } else {
        container->variant->setJsValue(jsValueContainer->jsValue);
    }
}

Q_DECL_EXPORT NetJSValueContainer* net_variant_getJsValue(NetVariantContainer* container) {
    QSharedPointer<NetJSValue> instance = container->variant->getJsValue();
    if(instance == nullptr) {
        return nullptr;
    }
    NetJSValueContainer* result = new NetJSValueContainer();
    result->jsValue = instance;
    return result;
}

Q_DECL_EXPORT void net_variant_clear(NetVariantContainer* container) {
    container->variant->clear();
}

}

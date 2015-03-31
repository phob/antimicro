#include <QHash>

#include "eventhandlerfactory.h"

static QHash<QString, QString> buildDisplayNames()
{
    QHash<QString, QString> temp;
    temp.insert("xtest", "Xtest");
    temp.insert("uinput", "uinput");
    return temp;
}

QHash<QString, QString> handlerDisplayNames = buildDisplayNames();

EventHandlerFactory* EventHandlerFactory::instance = 0;

EventHandlerFactory::EventHandlerFactory(QString handler, QObject *parent) :
    QObject(parent)
{
#ifdef Q_OS_UNIX
    #ifdef WITH_UINPUT
    if (handler == "uinput")
    {
        eventHandler = new UInputEventHandler(this);
    }
    #endif

    #ifdef WITH_XTEST
    if (handler == "xtest")
    {
        eventHandler = new XTestEventHandler(this);
    }
    #endif

#endif
}

EventHandlerFactory::~EventHandlerFactory()
{
    if (eventHandler)
    {
        delete eventHandler;
        eventHandler = 0;
    }
}

EventHandlerFactory* EventHandlerFactory::getInstance(QString handler)
{
    if (!instance)
    {
        QStringList temp = buildEventGeneratorList();
        if (temp.contains(handler))
        {
            instance = new EventHandlerFactory(handler);
        }
        else
        {
            instance = new EventHandlerFactory(fallBackIdentifier());
        }
    }

    return instance;
}

void EventHandlerFactory::deleteInstance()
{
    if (instance)
    {
        delete instance;
        instance = 0;
    }
}

BaseEventHandler* EventHandlerFactory::handler()
{
    return eventHandler;
}

QString EventHandlerFactory::fallBackIdentifier()
{
    QString temp;
#if defined(WITH_XTEST)
    temp = "xtest";
#elif defined(WITH_UINPUT)
    temp = "uinput";
#else
    temp = "xtest";
#endif

    return temp;
}

QStringList EventHandlerFactory::buildEventGeneratorList()
{
    QStringList temp;

    temp.append("xtest");
    temp.append("uinput");
    return temp;
}

QString EventHandlerFactory::handlerDisplayName(QString handler)
{
    QString temp;
    if (handlerDisplayNames.contains(handler))
    {
        temp = handlerDisplayNames.value(handler);
    }

    return temp;
}

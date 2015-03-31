#include <QTime>

#include "logger.h"

Logger* Logger::instance = 0;

/**
 * @brief Outputs log messages to a given text stream. Client code
 *     should determine whether it points to a console stream or
 *     to a file.
 * @param Stream used to output text
 * @param Messages based of a given output level or lower will be logged
 * @param Parent object
 */
Logger::Logger(QTextStream *stream, LogLevel outputLevel, QObject *parent) :
    QObject(parent)
{
    instance = this;
    instance->outputStream = stream;
    instance->outputLevel = outputLevel;
    instance->errorStream = 0;
    instance->pendingTimer.setInterval(0);
    instance->pendingTimer.setSingleShot(true);

    connect(&(instance->pendingTimer), SIGNAL(timeout()), instance, SLOT(Log()));
}

Logger::Logger(QTextStream *stream, QTextStream *errorStream, LogLevel outputLevel, QObject *parent) :
    QObject(parent)
{
    instance = this;
    instance->outputStream = stream;
    instance->outputLevel = outputLevel;
    instance->errorStream = errorStream;
    instance->pendingTimer.setInterval(0);
    instance->pendingTimer.setSingleShot(true);

    connect(&(instance->pendingTimer), SIGNAL(timeout()), instance, SLOT(Log()));
}

/**
 * @brief Close output stream and set instance to 0.
 */
Logger::~Logger()
{
    closeLogger();
}

/**
 * @brief Set the highest logging level. Determines which messages
 *     are output to the output stream.
 * @param Highest log level utilized.
 */
void Logger::setLogLevel(LogLevel level)
{
    Q_ASSERT(instance != 0);

    QMutexLocker locker(&instance->logMutex);
    Q_UNUSED(locker);

    instance->outputLevel = level;
}

Logger::LogLevel Logger::getCurrentLogLevel()
{
    Q_ASSERT(instance != 0);

    return instance->outputLevel;
}

void Logger::setCurrentStream(QTextStream *stream)
{
    Q_ASSERT(instance != 0);

    QMutexLocker locker(&instance->logMutex);
    Q_UNUSED(locker);

    instance->outputStream->flush();
    instance->outputStream = stream;
}

QTextStream* Logger::getCurrentStream()
{
    Q_ASSERT(instance != 0);

    return instance->outputStream;
}

void Logger::setCurrentErrorStream(QTextStream *stream)
{
    Q_ASSERT(instance != 0);

    QMutexLocker locker(&instance->logMutex);
    Q_UNUSED(locker);

    if (instance->errorStream)
    {
        instance->errorStream->flush();
    }

    instance->errorStream = stream;
}

QTextStream* Logger::getCurrentErrorStream()
{
    Q_ASSERT(instance != 0);

    return instance->errorStream;
}

/**
 * @brief Go through a list of pending messages and check if message should be
 *     logged according to the set log level. Log the message to the output
 *     stream.
 * @param Log level
 * @param String to write to output stream if appropriate to the current
 *     log level.
 */
void Logger::Log()
{
    QMutexLocker locker(&logMutex);
    Q_UNUSED(locker);

    QListIterator<LogMessage> iter(pendingMessages);
    while (iter.hasNext())
    {
        LogMessage pendingMessage = iter.next();

        LogLevel level = pendingMessage.level;
        QString message = pendingMessage.message;
        bool newline = pendingMessage.newline;

        if (outputLevel != LOG_NONE && level <= outputLevel)
        {
            QString displayTime = "";
            QString initialPrefix = "";
            if (outputLevel > LOG_INFO)
            {
                displayTime = QString("[%1] - ").arg(QTime::currentTime().toString("hh:mm:ss.zzz"));
                initialPrefix = displayTime;
            }

            QTextStream *writeStream = outputStream;
            if (level < LOG_INFO && errorStream)
            {
                writeStream = errorStream;
            }

            *writeStream << initialPrefix << message;
            if (newline)
            {
                *writeStream << endl;
            }

            writeStream->flush();
        }
    }

    pendingMessages.clear();
}

/**
 * @brief Flushes output stream and closes stream if requested.
 * @param Whether to close the current stream. Defaults to true.
 */
void Logger::closeLogger(bool closeStream)
{
    if (outputStream)
    {
        outputStream->flush();

        if (closeStream && outputStream->device() != 0)
        {
            QIODevice *device = outputStream->device();
            if (device->isOpen())
            {
                device->close();
            }
        }
    }

    if (errorStream)
    {
        errorStream->flush();

        if (closeStream && errorStream->device() != 0)
        {
            QIODevice *device = errorStream->device();
            if (device->isOpen())
            {
                device->close();
            }
        }
    }

    instance->pendingTimer.stop();
    instance = 0;
}

/**
 * @brief Append message to list of messages that might get placed in the
 *     log. Messages will be written later.
 * @param Log level
 * @param String to write to output stream if appropriate to the current
 *     log level.
 * @param Whether the logger should add a newline to the end of the message.
 */
void Logger::appendLog(LogLevel level, const QString &message, bool newline)
{
    Q_ASSERT(instance != 0);

    QMutexLocker locker(&instance->logMutex);
    Q_UNUSED(locker);

    LogMessage temp;
    temp.level = level;
    temp.message = QString(message);
    temp.newline = newline;

    instance->pendingMessages.append(temp);

    if (!instance->pendingTimer.isActive())
    {
        instance->pendingTimer.start();
    }
}

#ifndef ANTIMICROSETTINGS_H
#define ANTIMICROSETTINGS_H

#include <QSettings>

#include "commandlineutility.h"

class AntiMicroSettings : public QSettings
{
    Q_OBJECT
public:
    explicit AntiMicroSettings(const QString &fileName, Format format, QObject *parent = 0);

    QVariant runtimeValue(const QString &key, const QVariant &defaultValue = QVariant()) const;
    void importFromCommandLine(CommandLineUtility &cmdutility);

    static const bool defaultDisabledWinEnhanced;
    static const bool defaultAssociateProfiles;
    static const int defaultSpringScreen;

protected:
    QSettings cmdSettings;

signals:

public slots:

};

#endif // ANTIMICROSETTINGS_H

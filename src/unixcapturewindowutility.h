#ifndef UNIXCAPTUREWINDOWUTILITY_H
#define UNIXCAPTUREWINDOWUTILITY_H

#include <QObject>

class UnixCaptureWindowUtility : public QObject
{
    Q_OBJECT
public:
    explicit UnixCaptureWindowUtility(QObject *parent = 0);
    QString getTargetPath();
    bool hasFailed();

protected:
    QString targetPath;
    bool failed;

signals:
    void captureFinished();

public slots:
    void attemptWindowCapture();
};

#endif // UNIXCAPTUREWINDOWUTILITY_H

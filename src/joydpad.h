#ifndef JOYDPAD_H
#define JOYDPAD_H

#include <QObject>
#include <QHash>
#include <QString>
#include <QTimer>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "joybuttontypes/joydpadbutton.h"

class JoyDPad : public QObject
{
    Q_OBJECT
public:
    explicit JoyDPad(int index, int originset, SetJoystick *parentSet, QObject *parent=0);
    ~JoyDPad();

    enum JoyMode {StandardMode=0, EightWayMode, FourWayCardinal, FourWayDiagonal};

    JoyDPadButton* getJoyButton(int index);
    QHash<int, JoyDPadButton*>* getJoyButtons();

    int getCurrentDirection();
    int getJoyNumber();
    int getIndex();
    int getRealJoyNumber();
    virtual QString getName(bool fullForceFormat=false, bool displayNames=false);
    void joyEvent(int value, bool ignoresets=false);

    void setJoyMode(JoyMode mode);
    JoyMode getJoyMode();

    void releaseButtonEvents();

    void setButtonsMouseMode(JoyButton::JoyMouseMovementMode mode);
    bool hasSameButtonsMouseMode();
    JoyButton::JoyMouseMovementMode getButtonsPresetMouseMode();

    void setButtonsMouseCurve(JoyButton::JoyMouseCurve mouseCurve);
    bool hasSameButtonsMouseCurve();
    JoyButton::JoyMouseCurve getButtonsPresetMouseCurve();

    void setButtonsSpringWidth(int value);
    int getButtonsPresetSpringWidth();

    void setButtonsSpringHeight(int value);
    int getButtonsPresetSpringHeight();

    void setButtonsSensitivity(double value);
    double getButtonsPresetSensitivity();

    void setButtonsSmoothing(bool enabled=false);
    bool getButtonsPresetSmoothing();

    void setButtonsWheelSpeedX(int value);
    void setButtonsWheelSpeedY(int value);

    QString getDpadName();

    virtual bool isDefault();

    QHash<int, JoyDPadButton*>* getButtons();

    void readConfig(QXmlStreamReader *xml);
    void writeConfig(QXmlStreamWriter *xml);

    virtual QString getXmlName();

    virtual void setDefaultDPadName(QString tempname);
    virtual QString getDefaultDPadName();

    SetJoystick* getParentSet();
    bool hasSlotsAssigned();

    bool isRelativeSpring();
    void copyAssignments(JoyDPad *destDPad);

    unsigned int getDPadDelay();
    double getButtonsEasingDuration();

    static const QString xmlName;
    static const unsigned int DEFAULTDPADDELAY;

protected:
    void populateButtons();
    void createDeskEvent(bool ignoresets = false);
    QHash<int, JoyDPadButton*> getApplicableButtons();
    bool readMainConfig(QXmlStreamReader *xml);

    QHash<int, JoyDPadButton*> buttons;
    int index;
    JoyDPadButton::JoyDPadDirections prevDirection;
    JoyDPadButton::JoyDPadDirections pendingDirection;
    JoyDPadButton *activeDiagonalButton;
    int originset;
    JoyMode currentMode;
    QString dpadName;
    QString defaultDPadName;
    SetJoystick *parentSet;
    QTimer directionDelayTimer;
    unsigned int dpadDelay;

signals:
    void active(int value);
    void released(int value);
    void dpadNameChanged();
    void dpadDelayChanged(int value);
    void joyModeChanged();
    void propertyUpdated();

public slots:
    void setDPadName(QString tempName);
    void setButtonsSpringRelativeStatus(bool value);
    void setDPadDelay(int value);
    void setButtonsEasingDuration(double value);

    void establishPropertyUpdatedConnection();
    void disconnectPropertyUpdatedConnection();

private slots:
    void dpadDirectionChangeEvent();
};

#endif // JOYDPAD_H

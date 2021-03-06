#include "joycontrolstickpushbutton.h"
#include "joycontrolstickcontextmenu.h"

JoyControlStickPushButton::JoyControlStickPushButton(JoyControlStick *stick, bool displayNames, QWidget *parent) :
    FlashButtonWidget(displayNames, parent)
{
    this->stick = stick;

    refreshLabel();

    if (stick->getCurrentDirection() != JoyControlStick::StickCentered)
    {
        flash();
    }

    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showContextMenu(const QPoint&)));

    connect(stick, SIGNAL(active(int, int)), this, SLOT(flash()), Qt::QueuedConnection);
    connect(stick, SIGNAL(released(int, int)), this, SLOT(unflash()), Qt::QueuedConnection);
    connect(stick, SIGNAL(stickNameChanged()), this, SLOT(refreshLabel()));
}

JoyControlStick* JoyControlStickPushButton::getStick()
{
    return stick;
}

/**
 * @brief Generate the string that will be displayed on the button
 * @return Display string
 */
QString JoyControlStickPushButton::generateLabel()
{
    QString temp;
    if (!stick->getStickName().isEmpty() && displayNames)
    {
        temp.append(stick->getPartialName(false, true));
    }
    else
    {
        temp.append(stick->getPartialName(false));
    }

    return temp;
}

void JoyControlStickPushButton::disableFlashes()
{
    disconnect(stick, SIGNAL(active(int, int)), this, SLOT(flash()));
    disconnect(stick, SIGNAL(released(int, int)), this, SLOT(unflash()));
    this->unflash();
}

void JoyControlStickPushButton::enableFlashes()
{
    connect(stick, SIGNAL(active(int, int)), this, SLOT(flash()), Qt::QueuedConnection);
    connect(stick, SIGNAL(released(int, int)), this, SLOT(unflash()), Qt::QueuedConnection);
}

void JoyControlStickPushButton::showContextMenu(const QPoint &point)
{
    QPoint globalPos = this->mapToGlobal(point);
    JoyControlStickContextMenu *contextMenu = new JoyControlStickContextMenu(stick, this);
    contextMenu->buildMenu();
    contextMenu->popup(globalPos);
}

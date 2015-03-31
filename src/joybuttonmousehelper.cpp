//#include <QDebug>
#include <QList>

#include "joybutton.h"
#include "joybuttonmousehelper.h"

JoyButtonMouseHelper::JoyButtonMouseHelper(QObject *parent) :
    QObject(parent)
{
}

/**
 * @brief Perform mouse movement in cursor mode.
 */
void JoyButtonMouseHelper::moveMouseCursor()
{
    //qDebug() << QTime::currentTime();

    int finalx = 0;
    int finaly = 0;
    int elapsedTime = 5;
    JoyButton::moveMouseCursor(finalx, finaly, elapsedTime);
    if (finalx != 0 || finaly != 0)
    {
        emit mouseCursorMoved(finalx, finaly, elapsedTime);
    }
}

/**
 * @brief Perform mouse movement in spring mode.
 */
void JoyButtonMouseHelper::moveSpringMouse()
{
    int finalx = 0;
    int finaly = 0;
    bool hasMoved = false;
    JoyButton::moveSpringMouse(finalx, finaly, hasMoved);
    if (hasMoved)
    {
        emit mouseSpringMoved(finalx, finaly);
    }
}

/**
 * @brief Perform mouse events for all buttons and slots.
 */
void JoyButtonMouseHelper::mouseEvent()
{
    //qDebug() << "ENTER";

    if (!JoyButton::hasCursorEvents() && !JoyButton::hasSpringEvents())
    {
        //qDebug() << "GO THROUGH LIST";

        QList<JoyButton*> *fucker = JoyButton::getPendingMouseButtons();
        QListIterator<JoyButton*> iter(*fucker);
        while (iter.hasNext())
        {
            JoyButton *temp = iter.next();
            temp->mouseEvent();
        }
    }

    //if (JoyButton::hasCursorEvents())
    //{
        //qDebug() << "CURSOR EVENT: ";
        moveMouseCursor();
    //}

    if (JoyButton::hasSpringEvents())
    {
        moveSpringMouse();
    }
}

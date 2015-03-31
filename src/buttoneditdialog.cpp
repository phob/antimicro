//#include <QDebug>
#include <QtGlobal>

#ifdef Q_OS_WIN
#include <qt_windows.h>
#include "winextras.h"

#else

    #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QApplication>
    #endif
#endif

#include "buttoneditdialog.h"
#include "ui_buttoneditdialog.h"

#include "event.h"
#include "antkeymapper.h"

#ifdef Q_OS_UNIX
    #include "eventhandlerfactory.h"

    #if defined(WITH_UINPUT) && defined(WITH_X11)
        #include "qtx11keymapper.h"

        static QtX11KeyMapper x11KeyMapper;
    #endif
#endif

#include "setjoystick.h"

ButtonEditDialog::ButtonEditDialog(JoyButton *button, QWidget *parent) :
    QDialog(parent, Qt::Window),
    ui(new Ui::ButtonEditDialog)
{
    ui->setupUi(this);

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    setMinimumHeight(460);
#endif

    setAttribute(Qt::WA_DeleteOnClose);

    ignoreRelease = false;

    this->button = button;
    ui->virtualKeyMouseTabWidget->hide();
    ui->virtualKeyMouseTabWidget->deleteLater();
    ui->virtualKeyMouseTabWidget = new VirtualKeyboardMouseWidget(button, this);
    ui->virtualKeyMouseTabWidget->setFocus();

    ui->verticalLayout->insertWidget(1, ui->virtualKeyMouseTabWidget);
    ui->slotSummaryLabel->setText(button->getSlotsString());
    updateWindowTitleButtonName();

    ui->toggleCheckBox->setChecked(button->getToggleState());
    ui->turboCheckBox->setChecked(button->isUsingTurbo());

    if (!button->getActionName().isEmpty())
    {
        ui->actionNameLineEdit->setText(button->getActionName());
    }

    if (!button->getButtonName().isEmpty())
    {
        ui->buttonNameLineEdit->setText(button->getButtonName());
    }

    connect(qApp, SIGNAL(focusChanged(QWidget*,QWidget*)), this, SLOT(checkForKeyboardWidgetFocus(QWidget*,QWidget*)));

    connect(ui->virtualKeyMouseTabWidget, SIGNAL(selectionCleared()), this, SLOT(refreshSlotSummaryLabel()));
    connect(ui->virtualKeyMouseTabWidget, SIGNAL(selectionFinished()), this, SLOT(close()));

    connect(this, SIGNAL(keyGrabbed(JoyButtonSlot*)), this, SLOT(processSlotAssignment(JoyButtonSlot*)));
    connect(this, SIGNAL(selectionCleared()), this, SLOT(clearButtonSlots()));
    connect(this, SIGNAL(selectionCleared()), this, SLOT(sendSelectionFinished()));
    connect(this, SIGNAL(selectionFinished()), this, SLOT(close()));

    connect(ui->toggleCheckBox, SIGNAL(clicked()), this, SLOT(changeToggleSetting()));
    connect(ui->turboCheckBox, SIGNAL(clicked()), this, SLOT(changeTurboSetting()));
    connect(ui->advancedPushButton, SIGNAL(clicked()), this, SLOT(openAdvancedDialog()));
    connect(this, SIGNAL(advancedDialogOpened()), ui->virtualKeyMouseTabWidget, SLOT(establishVirtualKeyboardAdvancedSignalConnections()));
    connect(this, SIGNAL(advancedDialogOpened()), ui->virtualKeyMouseTabWidget, SLOT(establishVirtualMouseAdvancedSignalConnections()));
    connect(ui->virtualKeyMouseTabWidget, SIGNAL(selectionMade(int)), this, SLOT(createTempSlot(int)));

    connect(ui->actionNameLineEdit, SIGNAL(textEdited(QString)), button, SLOT(setActionName(QString)));
    connect(ui->buttonNameLineEdit, SIGNAL(textEdited(QString)), button, SLOT(setButtonName(QString)));

    connect(button, SIGNAL(toggleChanged(bool)), ui->toggleCheckBox, SLOT(setChecked(bool)));
    connect(button, SIGNAL(turboChanged(bool)), this, SLOT(checkTurboSetting(bool)));
    connect(button, SIGNAL(slotsChanged()), this, SLOT(refreshSlotSummaryLabel()));
    connect(button, SIGNAL(buttonNameChanged()), this, SLOT(updateWindowTitleButtonName()));
}

void ButtonEditDialog::checkForKeyboardWidgetFocus(QWidget *old, QWidget *now)
{
    Q_UNUSED(old);
    Q_UNUSED(now);

    if (ui->virtualKeyMouseTabWidget->hasFocus() &&
        ui->virtualKeyMouseTabWidget->isKeyboardTabVisible())
    {
        grabKeyboard();
    }
    else
    {
        releaseKeyboard();
    }
}

ButtonEditDialog::~ButtonEditDialog()
{
    delete ui;
}

void ButtonEditDialog::keyPressEvent(QKeyEvent *event)
{
    bool ignore = false;
    // Ignore the following keys that might
    // trigger an event in QDialog::keyPressEvent
    switch(event->key())
    {
        case Qt::Key_Escape:
        case Qt::Key_Right:
        case Qt::Key_Enter:
        case Qt::Key_Return:
        {
            ignore = true;
            break;
        }
    }

    if (!ignore)
    {
        QDialog::keyPressEvent(event);
    }
}

void ButtonEditDialog::keyReleaseEvent(QKeyEvent *event)
{
    if (ui->actionNameLineEdit->hasFocus() || ui->buttonNameLineEdit->hasFocus())
    {
        QDialog::keyReleaseEvent(event);
    }
    else if (ui->virtualKeyMouseTabWidget->isKeyboardTabVisible())
    {
        int controlcode = event->nativeScanCode();
        int virtualactual = event->nativeVirtualKey();

#ifdef Q_OS_WIN
        // Find more specific virtual key (VK_SHIFT -> VK_LSHIFT)
        // by checking for extended bit in scan code.
        int finalvirtual = WinExtras::correctVirtualKey(controlcode, virtualactual);
        int checkalias = AntKeyMapper::getInstance()->returnQtKey(finalvirtual, controlcode);

#else

        BaseEventHandler *handler = EventHandlerFactory::getInstance()->handler();

    #if defined(WITH_X11)
        int finalvirtual = 0;
        int checkalias = 0;

        #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
        if (QApplication::platformName() == QStringLiteral("xcb"))
        {
        #endif
        // Obtain group 1 X11 keysym. Removes effects from modifiers.
        finalvirtual = X11KeyCodeToX11KeySym(controlcode);

        #ifdef WITH_UINPUT
        if (handler->getIdentifier() == "uinput")
        {
            // Find Qt Key corresponding to X11 KeySym.
            checkalias = x11KeyMapper.returnQtKey(finalvirtual);
            // Find corresponding Linux input key for the Qt key.
            finalvirtual = AntKeyMapper::getInstance()->returnVirtualKey(checkalias);
        }
        #endif

        #ifdef WITH_XTEST
        if (handler->getIdentifier() == "xtest")
        {
            // Check for alias against group 1 keysym.
            checkalias = AntKeyMapper::getInstance()->returnQtKey(finalvirtual);
        }

        #endif

        #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
        }
        else
        {
            // Not running on xcb platform.
            finalvirtual = controlcode;
            checkalias = AntKeyMapper::getInstance()->returnQtKey(finalvirtual);
        }
        #endif

    #else
        int finalvirtual = 0;
        int checkalias = 0;
        #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
        if (QApplication::platformName() == QStringLiteral("xcb"))
        {
        #endif
        finalvirtual = AntKeyMapper::getInstance()->returnVirtualKey(event->key());
        checkalias = AntKeyMapper::getInstance()->returnQtKey(finalvirtual);
        #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
        }
        else
        {
            // Not running on xcb platform.
            finalvirtual = controlcode;
            checkalias = AntKeyMapper::getInstance()->returnQtKey(finalvirtual);
        }
        #endif

    #endif

#endif

        if (!ignoreRelease)
        {
            if ((event->modifiers() & Qt::ControlModifier) && event->key() == Qt::Key_X)
            {
                controlcode = 0;
                ignoreRelease = true;
                emit selectionCleared();
            }
            else if (controlcode <= 0)
            {
                controlcode = 0;
            }
        }
        else
        {
            controlcode = 0;
            ignoreRelease = false;
        }


        if (controlcode > 0)
        {
            if (checkalias > 0)
            {
                JoyButtonSlot *tempslot = new JoyButtonSlot(finalvirtual, checkalias, JoyButtonSlot::JoyKeyboard, this);
                emit keyGrabbed(tempslot);
            }
            else
            {
                JoyButtonSlot *tempslot = new JoyButtonSlot(virtualactual, JoyButtonSlot::JoyKeyboard, this);
                emit keyGrabbed(tempslot);
            }
        }
    }
    else
    {
        QDialog::keyReleaseEvent(event);
    }
}

void ButtonEditDialog::refreshSlotSummaryLabel()
{
    ui->slotSummaryLabel->setText(button->getSlotsString().replace("&", "&&"));
}

void ButtonEditDialog::changeToggleSetting()
{
    button->setToggle(ui->toggleCheckBox->isChecked());
}

void ButtonEditDialog::changeTurboSetting()
{
    button->setUseTurbo(ui->turboCheckBox->isChecked());
}

void ButtonEditDialog::openAdvancedDialog()
{
    ui->advancedPushButton->setEnabled(false);

    AdvanceButtonDialog *dialog = new AdvanceButtonDialog(button, this);
    dialog->show();

    // Disconnect event to allow for placing slot to AdvanceButtonDialog
    disconnect(this, SIGNAL(keyGrabbed(JoyButtonSlot*)), 0, 0);
    disconnect(this, SIGNAL(selectionCleared()), 0, 0);
    disconnect(this, SIGNAL(selectionFinished()), 0, 0);

    connect(dialog, SIGNAL(finished(int)), ui->virtualKeyMouseTabWidget, SLOT(establishVirtualKeyboardSingleSignalConnections()));
    connect(dialog, SIGNAL(finished(int)), ui->virtualKeyMouseTabWidget, SLOT(establishVirtualMouseSignalConnections()));
    connect(dialog, SIGNAL(finished(int)), this, SLOT(closedAdvancedDialog()));
    connect(dialog, SIGNAL(turboButtonEnabledChange(bool)), this, SLOT(setTurboButtonEnabled(bool)));

    connect(this, SIGNAL(sendTempSlotToAdvanced(JoyButtonSlot*)), dialog, SLOT(placeNewSlot(JoyButtonSlot*)));
    connect(this, SIGNAL(keyGrabbed(JoyButtonSlot*)), dialog, SLOT(placeNewSlot(JoyButtonSlot*)));
    connect(this, SIGNAL(selectionCleared()), dialog, SLOT(clearAllSlots()));
    connect(ui->virtualKeyMouseTabWidget, SIGNAL(selectionMade(JoyButtonSlot*)), dialog, SLOT(placeNewSlot(JoyButtonSlot*)));
    connect(ui->virtualKeyMouseTabWidget, SIGNAL(selectionCleared()), dialog, SLOT(clearAllSlots()));

    connect(this, SIGNAL(finished(int)), dialog, SLOT(close()));
    emit advancedDialogOpened();
}

void ButtonEditDialog::createTempSlot(int keycode)
{
    JoyButtonSlot *slot = new JoyButtonSlot(keycode, JoyButtonSlot::JoyKeyboard, this);
    emit sendTempSlotToAdvanced(slot);
}

void ButtonEditDialog::checkTurboSetting(bool state)
{
    if (button->containsSequence())
    {
        ui->turboCheckBox->setChecked(false);
        ui->turboCheckBox->setEnabled(false);
    }
    else
    {
        ui->turboCheckBox->setChecked(state);
        ui->turboCheckBox->setEnabled(true);
    }

    button->setUseTurbo(state);
}

void ButtonEditDialog::setTurboButtonEnabled(bool state)
{
    ui->turboCheckBox->setEnabled(state);
}

void ButtonEditDialog::closedAdvancedDialog()
{
    ui->advancedPushButton->setEnabled(true);

    // Re-connect previously disconnected event
    connect(this, SIGNAL(keyGrabbed(JoyButtonSlot*)), this, SLOT(processSlotAssignment(JoyButtonSlot*)));
    connect(this, SIGNAL(selectionCleared()), this, SLOT(clearButtonSlots()));
    connect(this, SIGNAL(selectionCleared()), this, SLOT(sendSelectionFinished()));
    connect(this, SIGNAL(selectionFinished()), this, SLOT(close()));
}

void ButtonEditDialog::processSlotAssignment(JoyButtonSlot *tempslot)
{
    button->clearSlotsEventReset(false);
    button->setAssignedSlot(tempslot->getSlotCode(), tempslot->getSlotCodeAlias(), tempslot->getSlotMode());
    this->close();
    tempslot->deleteLater();
}

void ButtonEditDialog::clearButtonSlots()
{
    button->clearSlotsEventReset();
}

void ButtonEditDialog::sendSelectionFinished()
{
    emit selectionFinished();
}

void ButtonEditDialog::updateWindowTitleButtonName()
{
    QString temp = QString(tr("Set")).append(" ").append(button->getPartialName(false, true));

    if (button->getParentSet()->getIndex() != 0)
    {
        unsigned int setIndex = button->getParentSet()->getRealIndex();
        temp.append(" [").append(tr("Set %1").arg(setIndex));
        QString setName = button->getParentSet()->getName();
        if (!setName.isEmpty())
        {
            temp.append(": ").append(setName);
        }

        temp.append("]");
    }

    setWindowTitle(temp);
}

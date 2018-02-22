
extern "C" {
#include "BSP_BoardType.h"
#include "BCDS_BSP_Button.h"
}

enum ButtonError {
    ButtonSuccess = 0,
    ButtonErrorInvalidButton,
    ButtonErrorConnect,
    ButtonErrorEnable,
};

void Button1Callback(uint32_t data);
void Button2Callback(uint32_t data);

// XXX: NASTY HACK
class Button;
Button *g_buttons[2] = {0};

/* microflo_component yaml
name: Button
description: Forward a packet from input to output
inports:
  enable:
    type: bool
    description: "Enable/disable reading of button"
  button:
    type: integer
    description: "Which button to read"
outports:
  out:
    type: bool
    description: "State of button"
  error:
    type: bool
    description: "Button setup error"
microflo_component */
class Button : public SingleOutputComponent {
public:
    Button()
        : button(-1)
        , initialized(false)
    {}

    virtual void process(Packet in, MicroFlo::PortId port) {
        using namespace ButtonPorts;
        if (port == InPorts::button) {
            button = in.asInteger();
        } else if (port == InPorts::enable) {
            const bool enabled = in.asBool();
            // TODO: implement disable

            const ButtonError err = enableButtonRead();
            if (err == ButtonSuccess) {
                //send(true, OutPorts::out);
            } else {
                send((long)err, OutPorts::error);
            }
        }
    }

    void sendData(bool s) {
        send(s, ButtonPorts::OutPorts::out);
    }

private:
    ButtonError enableButtonRead() {
        if (button < 0) {
            return ButtonErrorInvalidButton;
        }
        if (button > 2) {
            return ButtonErrorInvalidButton;
        }

        if (initialized) {
            // Already initialized
            return ButtonSuccess;
        }

        const Retcode_T connected = BSP_Button_Connect();
        if (RETCODE_OK == connected) {
            return ButtonErrorConnect;
        }
        const Retcode_T enabled = BSP_Button_Enable((uint32_t)button,
                                        (button == 1) ? Button1Callback : Button2Callback);
        if (RETCODE_OK == enabled) {
            return ButtonErrorConnect;
        }
        g_buttons[button-1] = this;

        return ButtonSuccess;
    }
private:
    int8_t button;
    bool initialized;
};


void Button1Callback(uint32_t data)
{
    Button *b = g_buttons[0];
    if (b) {
        b->sendData(data == BSP_XDK_BUTTON_PRESS);
    }
}
void Button2Callback(uint32_t data)
{
    Button *b = g_buttons[1];
    if (b) {
        b->sendData(data == BSP_XDK_BUTTON_PRESS);
    }
}


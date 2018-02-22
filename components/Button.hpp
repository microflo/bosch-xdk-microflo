

enum ButtonError {
    ButtonSuccess = 0,
    ButtonErrorInvalidButton,
    ButtonErrorConnect,
    ButtonErrorEnable,
};

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
                send(true, OutPorts::out);
            } else {
                send((long)err, OutPorts::error);
            }
        }
    }

private:
    ButtonError enableButtonRead() {


    }
private:
    int8_t button;
};

#if 0
    Retcode_T returnVal = RETCODE_OK;
    returnVal = BSP_Button_Connect();
    if (RETCODE_OK == returnVal)
    {
        returnVal = BSP_Button_Enable((uint32_t) BSP_XDK_BUTTON_1, Button1Callback);
    }

void ButtonCallback(uint32_t data)
{
    Retcode_T returnValue = CmdProcessor_EnqueueFromIsr(AppCmdProcessor, processButton1Data, NULL, data);
    if (RETCODE_OK != returnValue)
    {
        printf("Enqueuing for Button 1 callback failed\n\r");
    }
}

#endif

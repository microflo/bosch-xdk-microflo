
extern "C" {
    #include "BSP_BoardType.h"
    #include "BCDS_BSP_LED.h"
}

enum LedError {
    LedSuccess = 0,
    LedErrorInvalidLed,
    LedErrorConnect,
    LedErrorEnable,
    LedErrorSet,
};

/* microflo_component yaml
name: Led
description: Forward a packet from input to output
inports:
  in:
    type: boolean
    description: "LED on/off"
  pin:
    type: integer
    description: "Which LED to control (1,2,3)"
outports:
  out:
    type: boolean
    description: "Confirmation of @in"
  error:
    type: all
    description: "Confirmation of @in"
microflo_component */
class Led : public SingleOutputComponent {
public:
    Led()
        : led(-1)
        , initialized(false)
    {
    }

    virtual void process(Packet in, MicroFlo::PortId port) {
        using namespace LedPorts;

        if (port == InPorts::pin) {
            this->led = in.asInteger();
        } else if (port == InPorts::in) {
            const bool state = in.asBool();
            const LedError err = setLed(state);
            if (err == LedSuccess) {
                send(state, OutPorts::out);
            } else {
                send((long)err, OutPorts::error);
            }
        }
    }

private:
    LedError setLed(bool state) {
        if (led <= 0) {
            return LedErrorInvalidLed;
        }
        if (led > 3) {
            return LedErrorInvalidLed;
        }

        if (!initialized) {
            const Retcode_T connected = BSP_LED_Connect();
            if (RETCODE_OK != connected) {
                return LedErrorConnect;
            }
            const Retcode_T enabled = BSP_LED_Enable((uint32_t)led);
            if (RETCODE_OK != enabled) {
                return LedErrorEnable;
            }
            initialized = true;
        }

        const uint32_t cmd = (state) ? BSP_LED_COMMAND_ON : BSP_LED_COMMAND_OFF; 
        const Retcode_T switched = BSP_LED_Switch(led, cmd);
        if (RETCODE_OK != switched) {
            return LedErrorSet;
        }

        return LedSuccess;
    }

private:
    int8_t led;
    bool initialized;
};

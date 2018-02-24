/* microflo_component yaml
name: Timer
description: Fire event every @millis
icon: "clock-o"
inports:
  enable:
    type: boolean
    description: "Enable/disable the timer"
  millis:
    type: integer
    description: "How often to fire"
outports:
  out:
    type: bang
    description: ""
microflo_component */
class Timer : public SingleOutputComponent {
public:
    virtual void process(Packet in, MicroFlo::PortId port) {
        if (in.isData()) {
            send(in, port);
        }
    }
};

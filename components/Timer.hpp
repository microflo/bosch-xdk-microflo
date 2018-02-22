/* microflo_component yaml
name: Timer
description: Forward a packet from input to output
inports:
  enable:
    type: boolean
    description: "Enable/disable the timer"
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

/* microflo_component yaml
name: OutputNumber
description: Emits number on each tick
inports:
  in:
    type: number
    description: ""
outports:
  out:
    type: number
    description: ""
microflo_component */
class OutputNumber : public SingleOutputComponent {
public:
    virtual void process(Packet in, MicroFlo::PortId port) { 
        if (in.isTick()) {
            send((long)666, OutputNumberPorts::OutPorts::out);
        }
    }
};

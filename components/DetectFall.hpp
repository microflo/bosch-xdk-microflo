/* microflo_component yaml
name: DetectFall
description: Forward a packet from input to output
inports:
  x:
    type: all
    description: ""
  y:
    type: all
    description: ""
  z:
    type: all
    description: ""
outports:
  out:
    type: all
    description: ""
microflo_component */
class DetectFall : public SingleOutputComponent {
public:
    virtual void process(Packet in, MicroFlo::PortId port) {
        if (in.isData()) {
            send(in, port);
        }
    }
};

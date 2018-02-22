/* microflo_component yaml
name: ReadAccelerometer
description: Forward a packet from input to output
inports:
  in:
    type: bang
    description: ""
outports:
  x:
    type: integer
    description: ""
  y:
    type: integer
    description: ""
  z:
    type: integer
    description: ""
microflo_component */
class ReadAccelerometer : public SingleOutputComponent {
public:
    virtual void process(Packet in, MicroFlo::PortId port) {
        if (in.isData()) {
            send(in, port);
        }
    }
};

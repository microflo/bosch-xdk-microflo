/* microflo_component yaml
name: Print
description: Prints to stdout
inports:
  in:
    type: number
    description: ""
outports:
  out:
    type: number
    description: ""
microflo_component */
class Print : public SingleOutputComponent {
public:
    virtual void process(Packet in, MicroFlo::PortId port) {
        if (in.isData()) {

            printf("Print: %d\n\r", in.asInteger());

            send(in, port);
        }
    }
};

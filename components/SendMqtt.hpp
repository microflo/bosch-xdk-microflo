/* microflo_component yaml
name: SendMqtt
description: Send message on MQTT
icon: "send-o"
inports:
  in:
    type: all
    description: ""
outports:
  out:
    type: all
    description: ""
microflo_component */
class SendMqtt : public SingleOutputComponent {
public:
    virtual void process(Packet in, MicroFlo::PortId port) {
        if (in.isData()) {
            send(in, port);
        }
    }
};

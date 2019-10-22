#include <iostream>
#include "MessageType.h"

class ActorTest :public orca::Actor
{
public:
    ActorTest(orca::Framework* framework,std::string name = "")
        :Actor(framework, name)
    {
        registerHandler(std::bind(&ActorTest::handle,this,std::placeholders::_1,std::placeholders::_2));
    }
    void handle(orca::MessagePack& pack, orca::Address& from)
    {
        std::cout << (char*)(pack.enter()) << std::endl;
    }
};

int main(int argc, char** args)
{
    //actor framework.
    orca::Framework framework;
    //arctor object.
    ActorTest actor1(&framework);
    ActorTest actor2(&framework);
    //message pack.
    char data[] = "a message of my customize type";
    orca::MessagePack message;
    message.create(data,(int)sizeof(data));

    //actor1->actor2 send message.
    actor1.send(message,actor2.getAddress());
    framework.loop();

}
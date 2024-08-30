//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "AgentApp.h"

#include <math.h>

#include "inet/applications/base/ApplicationPacket_m.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/TagBase_m.h"
#include "inet/common/TimeTag_m.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/networklayer/common/FragmentationTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/transportlayer/udp/Udp.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo_m.h"

#include "untitled_m.h"

//#include "carla_omnet/TodCarlanetManager.h"
//#include "messages/TodMessages_m.h"

using namespace omnetpp;
using namespace inet;

Define_Module(AgentApp);

AgentApp::~AgentApp()
{
    //cancelAndDelete(updateStatusSelfMessage);
}

void AgentApp::initialize(int stage)
{
    ApplicationBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        carlanetManager = check_and_cast<CarlanetManager*>(getParentModule()->getParentModule()->getSubmodule("carlanetManager"));
        firstCommandMsg = new cMessage("firstCommandMsg");
        commandStartTime = par("sendInterval");
    }
}

void AgentApp::refreshDisplay() const{}

void AgentApp::finish()
{
    ApplicationBase::finish();
}

void AgentApp::handleStartOperation(LifecycleOperation *operation)
{
    L3AddressResolver().tryResolve(par("destAddress"), destAddress);
    destPort = par("destPort");

    socket.setOutputGate(gate("socketOut"));
    socket.bind(destPort);
    socket.setCallback(this);

    simtime_t firstCommandTime = simTime() + carlanetManager->getCarlaInitialCarlaTimestamp() + commandStartTime;
    scheduleAt(firstCommandTime, firstCommandMsg);
}


void AgentApp::handleStopOperation(LifecycleOperation *operation)
{
    socket.close();
}

void AgentApp::handleCrashOperation(LifecycleOperation *operation)
{
    if (operation->getRootModule() != getContainingNode(this)) // closes socket when the application crashed only
        socket.destroy(); // TODO  in real operating systems, program crash detected by OS and OS closes sockets of crashed programs.
    socket.setCallback(nullptr);
}


void AgentApp::handleMessageWhenUp(cMessage* msg){

    if (msg->isSelfMessage()){

        if (msg == firstCommandMsg){
            sendNewLightCommand();
            //sendUpdateStatusPacket();
            // this time contains all the parameters needed to generate status message, TODO: create ad hoc message
            // Note, you have to add the same time for all the UDP pkt of one frame
            //scheduleAfter(commandUpdateInterval, msg);

        }
    }else if(socket.belongsToSocket(msg)){
            socket.processMessage(msg);
    }

}


void AgentApp::socketDataArrived(UdpSocket *socket, Packet *packet){
    emit(packetReceivedSignal, packet);
    EV_INFO << "Received packet: " << UdpSocket::getReceivedPacketInfo(packet) << endl;

    processPacket(packet);

    delete packet;
    numReceived++;
}


void AgentApp::socketErrorArrived(UdpSocket *socket, Indication *indication){}


void AgentApp::socketClosed(UdpSocket *socket){}

void AgentApp::sendPacket(Packet *packet){
    emit(packetSentSignal, packet);
    socket.sendTo(packet, destAddress, destPort);
    //numSent++;
}



void AgentApp::processPacket(Packet *pk){
    if (pk->hasData<LightStatusMessage2>()){
        auto message_sp = pk->peekData<LightStatusMessage2>();
        EV_INFO << "Received a new light status: " << message_sp->getLightCurrState() << endl;

        current_light_state = message_sp->getLightCurrState();
        sendNewLightCommand();
    }
    else {
        EV_WARN << "Received an unexpected packet "<< UdpSocket::getReceivedPacketInfo(pk) <<endl;
    }
}

void AgentApp::sendNewLightCommand(){
    EV_INFO << "Send status update" << endl;

    try {
        light_update lightStateMsg;
        lightStateMsg.light_curr_state = current_light_state;

        auto lightCommandMsg = carlanetManager->sendToAndGetFromCarla_agent_generic_message<light_update, light_command>(lightStateMsg);
        const int fragmentLength = std::min((int) par("messageLength"), (int) UDP_MAX_MESSAGE_SIZE-10);
        auto packet = new Packet("LightCommand_");
        auto data = makeShared<LightCommandMessage2>();
        data->setChunkLength(B(fragmentLength));
        data->setLightNextState(lightCommandMsg.light_next_state.c_str());
        auto creationTimeTag = data->addTag<CreationTimeTag>(); // add new tag
        creationTimeTag->setCreationTime(simTime()); // store current time
        packet->insertAtBack(data);
        socket.sendTo(packet, destAddress, destPort);
    } catch (const std::exception& e) {
        std::cerr << "An exception occurred: " << e.what() << std::endl;
    }


}


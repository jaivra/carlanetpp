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

#ifndef __CARLANETPP_CARAPP_H_
#define __CARLANETPP_CARAPP_H_

#include <vector>
#include <omnetpp.h>

#include "inet/networklayer/common/L3Address.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"
#include "inet/applications/base/ApplicationBase.h"

#include "../CarlanetManager.h"
#include "../lightcontrol/CarlaMessages.h"

using namespace omnetpp;
using namespace inet;

/**
 * TODO - Generated class
 */
class CarApp : public ApplicationBase, public UdpSocket::ICallback
{

private:
    CarlanetManager* carlanetManager;
    //cMessage* updateStatusSelfMessage;


protected:
    UdpSocket socket;
    L3Address destAddress;
    int destPort;
    // statistics



protected:
    virtual int numInitStages() const override { return inet::NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void finish() override;
    virtual void refreshDisplay() const override;

    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;



    /*Application logic*/
    //virtual void sendUpdateStatusPacket(simtime_t dataRetrievalTime);

    /*UDP logic*/
    /**
     * Notifies about data arrival, packet ownership is transferred to the callee.
     */
    virtual void socketDataArrived(UdpSocket *socket, Packet *packet);

    /**
     * Notifies about error indication arrival, indication ownership is transferred to the callee.
     */
    virtual void socketErrorArrived(UdpSocket *socket, Indication *indication);

    /**
     * Notifies about socket closed, indication ownership is transferred to the callee.
     */
    virtual void socketClosed(UdpSocket *socket);


    virtual void sendPacket(Packet *pk);
    virtual void processPacket(Packet *pk);

public:
    ~CarApp();


};


#endif

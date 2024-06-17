// MIT License
// Copyright (c) 2023 Valerio Cislaghi, Christian Quadri

/*
 * Class that implements the logic of the communication between CARLA (pyCARLANeT) and OMNeT++ (CARLANeTpp)
 */

#ifndef __CARLANETMANAGER_H_
#define __CARLANETMANAGER_H_

#include <string>
#include <list>
#include <zmq.hpp>
#include <iostream>
#include <chrono>
#include <thread>

#include <map>
#include <memory>
#include <list>
#include <queue>
#include <fstream>

#include "omnetpp.h"

#include "carlaApi.h"
#include "CarlaInetMobility.h"
#include "inet/common/INETDefs.h"

using namespace std;
using namespace omnetpp;
using namespace inet;



class CarlanetManager: public cSimpleModule {
public:
    CarlanetManager();
    ~CarlanetManager();

    bool isConnected() const
    {
        return true;
    }

    simtime_t getCarlaInitialCarlaTimestamp() { return initial_timestamp; }


    void registerMobilityModule(CarlaInetMobility *mod);


protected:
    virtual int numInitStages() const override { return inet::NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;

    /**
     * This base implementation gets CARLA extra initialization parameters directly from
     * extraInitParams parameters of the module. This method could be override to be more
     * tailored to specific configuration and parameters' set
     */
    virtual const std::map<std::string,cValue>& getExtraInitParams();

private:
    void connect();
    void doSimulationTimeStep();
    void initializeCarla();
    void findModulesToTrack();
    void updateNodesPosition(std::list<carla_api_base::actor_position> actor);

    void sendToCarla(json jsonMsg){
        std::stringstream msg;
        socket.send(zmq::buffer(jsonMsg.dump()), zmq::send_flags::none);
    }

    json receiveFromCarla(double timeoutFactor);

    template <typename T> T receiveFromCarla(double timeoutFactor = 1){
        return receiveFromCarla(timeoutFactor).get<T>();
    }

    bool connection;
    string protocol;
    string host;
    double simulationTimeStep;
    simtime_t initial_timestamp = 0;
    int port;
    zmq::context_t context;
    zmq::socket_t socket;
    int timeout_ms;
    cMessage *simulationTimeStepEvent =  new cMessage("simulationTimeStep");
    map<string,CarlaInetMobility*> modulesToTrack = map<string,CarlaInetMobility*>();
    //map<string,CarlaInetMobility*> modulesToTrack = map<string,CarlaInetMobility*>();


    //Handlers for dynamic actor creation/destroying
    void createAndInitializeActor(carla_api_base::actor_position newActor);
    void destroyActor(string actorId);
    const char* networkActiveModuleType;
    const char* networkPassiveModuleType;


public:
    //API used by applications
    /**
     * This is a generic API that accepts and return json type
     */
    json sendToAndGetFromCarla(json requestMessage){
        carla_api::generic_message toCarlaMessage;
        toCarlaMessage.user_defined = requestMessage;
        toCarlaMessage.timestamp = simTime().dbl();

        json jsonMsg = toCarlaMessage;
        sendToCarla(jsonMsg);

        auto jsonResp = receiveFromCarla<carla_api::generic_response>(10.0);
        return jsonResp.user_defined;

    }

    /*
     * Variants of the API using templates
     * IMPORTANT the type must implement "to_json" and "from_json" as specified
     * by the "nlohmann/json" library
     */
    template<typename S, typename T> T sendToAndGetFromCarla(S requestMessage){
        json jsonRequestMessage = requestMessage;
        json jsonResponseMessage = sendToAndGetFromCarla(jsonRequestMessage);
        return jsonResponseMessage.get<T>();
    }

    // WORLD GENERIC MESSAGE
    template<typename S, typename T> T sendToAndGetFromCarla_world_generic_message(S requestMessage){
        json jsonRequestMessage = requestMessage;
        json jsonResponseMessage = sendToAndGetFromCarla_world_generic_message(jsonRequestMessage);
        return jsonResponseMessage.get<T>();
    }
    json sendToAndGetFromCarla_world_generic_message(json requestMessage){
        carla_api::world_generic_message toCarlaMessage;
        toCarlaMessage.user_defined = requestMessage;
        toCarlaMessage.timestamp = simTime().dbl();
        json jsonMsg = toCarlaMessage;
        EV << "socket -> " << jsonMsg.dump() <<  endl;
        sendToCarla(jsonMsg);
        auto jsonResp = receiveFromCarla<carla_api::world_generic_response>(10.0);
        json res = jsonResp;
        EV << "socket <- " << res.dump() <<  endl;
        return jsonResp.user_defined;
    }

};

#endif

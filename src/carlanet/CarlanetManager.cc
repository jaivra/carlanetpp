// MIT License
// Copyright (c) 2023 Valerio Cislaghi, Christian Quadri


#include "CarlanetManager.h"

#include <stdexcept>

#include "inet/applications/base/ApplicationPacket_m.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/TagBase_m.h"
#include "inet/common/TimeTag_m.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/FragmentationTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo_m.h"
#include "inet/common/scenario/ScenarioManager.h"

#include <thread>
#include <chrono>


Define_Module(CarlanetManager);

using namespace inet;
using namespace std;

CarlanetManager::CarlanetManager(){

}
CarlanetManager::~CarlanetManager(){
    cancelAndDelete(simulationTimeStepEvent);
}


void CarlanetManager::finish(){

}


void CarlanetManager::initialize(int stage)
{
    cSimpleModule::initialize(stage);
    if (stage == INITSTAGE_LOCAL){
        protocol = par("protocol").stdstringValue();
        host = par("host").stdstringValue();
        port = par("port").intValue();
        timeout_ms = par("communicationTimeoutms");
        simulationTimeStep = par("simulationTimeStep");

        moduleType = check_and_cast<cValueMap*>(par("moduleType").objectValue())->getFields();
        moduleName = check_and_cast<cValueMap*>(par("moduleName").objectValue())->getFields();

        connect();
    }

    if (stage == INITSTAGE_SINGLE_MOBILITY){
        initializeCarla();
    }
}


void CarlanetManager::registerMobilityModule(CarlaInetMobility *mod){
    modulesToTrack.insert(pair<string,CarlaInetMobility*>(mod->getCarlaId(), mod));
    //std::cout << "registerMobilityModule "<< mod->getCarlaId() << " " << mod->getCarlaActorType() <<  endl;
}


void CarlanetManager::initializeCarla(){
    // conversion
    auto movingActorList = list<carla_api_base::init_actor>();
    for(auto elem: modulesToTrack){
        std::cout << "elem: modulesToTrack " << static_cast<std::string>(elem.first) <<" " << static_cast<std::string>(elem.second->getCarlaActorType()) <<  endl;
        carla_api_base::init_actor actor;
        actor.actor_id = elem.first;
        actor.actor_type = elem.second->getCarlaActorType();
        actor.actor_configuration = elem.second->getCarlaActorConfiguration()->getFields();
        movingActorList.push_back(actor);
    }

    auto simTimeLimit = getEnvir()->getConfigEx()->getConfigValue("sim-time-limit");


    // compose the message
    carla_api::init msg;
    msg.run_id = getEnvir()->getConfigEx()->getVariable(CFGVAR_RUNID);

    msg.carla_configuration.seed = stoi(getEnvir()->getConfigEx()->getVariable(CFGVAR_SEEDSET));
    msg.carla_configuration.carla_timestep = simulationTimeStep;
    msg.carla_configuration.sim_time_limit = simTimeLimit != nullptr ? stod(simTimeLimit) : -1.0 ;
    msg.moving_actors = movingActorList;
    msg.user_defined = getExtraInitParams();
    msg.timestamp = simTime().dbl();

    std::vector<std::string> intersection;
    for (const auto& e : moduleType){
        if (moduleName.find(e.first) != moduleName.end()) {
            intersection.push_back(e.first);
        }
    }
    msg.actor_types = intersection;

    json jsonMsg = msg;

    EV << jsonMsg.dump() << endl;
    sendToCarla(jsonMsg);
    // I expect to receive INIT_COMPLETE message
    carla_api::init_completed response = receiveFromCarla<carla_api::init_completed>(100.0);
    // Carla informs about the intial timestamp, so I schedule the first similation step at that timestamp
    EV << "Initialization completed " << response.initial_timestamp <<  endl;
    updateNodesPosition(response.actor_positions);
    //
    initial_timestamp = simTime() + response.initial_timestamp;
    // schedule
    scheduleAt(simTime() + response.initial_timestamp, simulationTimeStepEvent);
}

const std::map<std::string,cValue>& CarlanetManager::getExtraInitParams(){
    return check_and_cast<cValueMap*>(par("extraInitParams").objectValue())->getFields();
}


void CarlanetManager::doSimulationTimeStep(){
    carla_api::simulation_step msg;
    msg.carla_timestep = simulationTimeStep;
    msg.timestamp = simTime().dbl();
    json jsonMsg = msg;
    EV << "socket -> " << jsonMsg.dump() <<  endl;
    sendToCarla(jsonMsg);
    // I expect updated_postion message
    carla_api::updated_postion response = receiveFromCarla<carla_api::updated_postion>();

    json jsonResponse = response;
    EV << "socket <-" << jsonResponse.dump() << endl;

    //Update position of all nodes in response
    updateNodesPosition(response.actor_positions);
}

void CarlanetManager::updateNodesPosition(std::list<carla_api_base::actor_position> actors){
    set<string> knownActors = set<string>();
    for(auto const& item: modulesToTrack)
        knownActors.insert(item.first);

    // Update the mobility of actors or create new ones in they do not exist
    for(auto const &actor : actors){
        if (knownActors.find(actor.actor_id) == knownActors.end()){  //NOT FOUND
            createAndInitializeActor(actor);
        }
        else{
            knownActors.erase(actor.actor_id);  //OK Found I can update it
        }

        Coord position = Coord(actor.position[0], actor.position[1], actor.position[2]);
        Coord velocity = Coord(actor.velocity[0],actor.velocity[1],actor.velocity[2]);
        Quaternion rotation = Quaternion(EulerAngles(rad(actor.rotation[0]),rad(actor.rotation[1]),rad(actor.rotation[2])));
        modulesToTrack[actor.actor_id]->nextPosition(position, velocity, rotation);

    }

    // remove actors which where known but CARLA has just destroyed
    for (auto const &actorId : knownActors){
        destroyActor(actorId);
    }

}



void CarlanetManager::connect(){
    this->context = zmq::context_t {1};
    this->socket = zmq::socket_t{context, zmq::socket_type::req};

    this->socket.setsockopt(ZMQ_RCVTIMEO, timeout_ms); // set timeout to value of timeout_ms
    this->socket.setsockopt(ZMQ_SNDTIMEO, timeout_ms); // set timeout to value of timeout_ms
    EV_INFO << "CarlaCommunicationManagerLog " << "Finish initialize" << endl;
    string addr = protocol + "://" + host + ":" + std::to_string(port);
    EV << "Trying connecting to: " << addr << endl;
    socket.connect(addr);
}

void CarlanetManager::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()){
        if (msg == simulationTimeStepEvent){
            doSimulationTimeStep();
            EV_INFO << "Simulation step: " << this->simulationTimeStep << endl;
            scheduleAt(simTime() + this->simulationTimeStep, msg);
        }
    }
}

/* ***********************************
 * Dynamic creation/destroying actors
 * ********************************** */
void CarlanetManager::createAndInitializeActor(carla_api_base::actor_position newActor){
    //auto newActorModuleType = moduleType[];
    //auto newActorModuleName = newActor.is_net_active ? networkActiveModuleName : networkPassiveModuleName;
    auto posModuleType = moduleType.find(newActor.type);
    std::string newActorModuleType = posModuleType->second.stringValue();

    auto posModuleName = moduleName.find(newActor.type);
    std::string newActorModuleName = posModuleName->second.stringValue();

    std::cout << "module type and name: " << newActorModuleType.c_str() << " - " << newActorModuleName.c_str() << endl;

    cModule* root = getSimulation()->getSystemModule();
    int i = root->getSubmoduleVectorSize(newActorModuleName.c_str());
    root->setSubmoduleVectorSize(newActorModuleName.c_str(), i+1);
    cModuleType *actorType = cModuleType::get(newActorModuleType.c_str());
    cModule* new_mod = actorType->create(newActorModuleName.c_str(), root, i);

    new_mod->finalizeParameters();

    new_mod->buildInside();

    new_mod->scheduleStart(simTime());

    // Pre initialize mobility
    Coord position = Coord(newActor.position[0], newActor.position[1], newActor.position[2]);
    Coord velocity = Coord(newActor.velocity[0],newActor.velocity[1],newActor.velocity[2]);
    Quaternion rotation = Quaternion(EulerAngles(rad(newActor.rotation[0]),rad(newActor.rotation[1]),rad(newActor.rotation[2])));
    auto CarlaInetMobilityMod = check_and_cast<CarlaInetMobility *>(new_mod->getSubmodule("mobility"));
    CarlaInetMobilityMod->preInitialize(newActor.actor_id, newActor.type, position, velocity, rotation);

    // The INET visualizer listens to model change notifications on the
    // network object by default. We assume this is our parent.
    auto* notification = new inet::cPreModuleInitNotification();
    notification->module = new_mod;
    root->emit(POST_MODEL_CHANGE, notification, NULL);

    new_mod->callInitialize();

    std::cout << "Created module: " << new_mod->getFullPath() << endl;

}


void CarlanetManager::destroyActor(string actorId){
    //NOTE the map contains the reference to the mobilityModule
    // This implementation assumes that mobility module is a direct child of the actor module
    auto mod = modulesToTrack[actorId]->getParentModule();

    mod->callFinish();
    mod->deleteModule();

    modulesToTrack.erase(actorId);

}


json CarlanetManager::receiveFromCarla(double timeoutFactor){
    // set actual timeout
    int recv_timeout_ms =  max(4000, int(timeout_ms * timeoutFactor));
    this->socket.setsockopt(ZMQ_RCVTIMEO, recv_timeout_ms);

    zmq::message_t reply{};

    //assert(!socket.recv(reply, zmq::recv_flags::none));
    if (!socket.recv(reply, zmq::recv_flags::none)){
        throw runtime_error("CALRA Timeout");
        //EV_ERROR << "receive error"<<endl;
    }
    json jsonResp = json::parse(reply.to_string());

    switch (jsonResp["simulation_status"].get<int>()){
    case SIM_STATUS_FINISHED_OK:
    case SIM_STATUS_FINISHED_ACCIDENT:
    case SIM_STATUS_FINISHED_TIME_LIMIT:
        endSimulation();
        break;
    case SIM_STATUS_ERROR:
        throw runtime_error("Communication error. Wrong message sequence!");
        break;
    }
    return jsonResp;
}





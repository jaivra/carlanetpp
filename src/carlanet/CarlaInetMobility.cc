// MIT License
// Copyright (c) 2023 Valerio Cislaghi, Christian Quadri


#include "CarlaInetMobility.h"

#include "CarlanetManager.h"

#include "utils.h"

Define_Module(CarlaInetMobility);

void CarlaInetMobility::initialize(int stage)
{
    MobilityBase::initialize(stage);

    if (stage == inet::INITSTAGE_LOCAL){
        carlaActorType = par("carlaActorType").stdstringValue();
        carlaActorConfiguration = check_and_cast<cValueMap*>(par("carlaActorConfiguration").objectValue()); //.cValueMap(); // .objectValue();
        updateCarlaActorConfigurationFromParam(carlaActorConfiguration);
        auto carlaManager = getFirstSubmoduleOfType<CarlanetManager>(getModuleByPath("<root>"));
        // register to carlaManager
        carlaManager->registerMobilityModule(this);
    }
}

void CarlaInetMobility::setInitialPosition(){
    if (!preInitialized){
        MobilityBase::setInitialPosition();
    }
}

void CarlaInetMobility::preInitialize(const std::string carlaID, const inet::Coord& position, const inet::Coord& velocity,  const inet::Quaternion& rotation){
    preInitialized = true;
    carla_id = carlaID;
    lastPosition = position;
    lastVelocity = velocity;
    lastOrientation = rotation;

}


void CarlaInetMobility::nextPosition(const inet::Coord& position, const inet::Coord& velocity,  const inet::Quaternion& rotation){
    lastPosition = position;
    lastVelocity = velocity;
    lastOrientation = rotation;

    emitMobilityStateChangedSignal();
}


const inet::Coord& CarlaInetMobility::getCurrentPosition()
{
    return lastPosition;
}

const inet::Coord& CarlaInetMobility::getCurrentVelocity()
{
    return lastVelocity;
}

const inet::Coord& CarlaInetMobility::getCurrentAcceleration()
{
    throw cRuntimeError("Invalid operation");
}

const inet::Quaternion& CarlaInetMobility::getCurrentAngularPosition()
{
    return lastOrientation;
}

const inet::Quaternion& CarlaInetMobility::getCurrentAngularVelocity()
{
    return lastAngularVelocity;
}

const inet::Quaternion& CarlaInetMobility::getCurrentAngularAcceleration()
{
    throw cRuntimeError("Invalid operation");
}


void CarlaInetMobility::handleSelfMessage(cMessage* msg){
}

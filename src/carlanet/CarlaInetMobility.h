// MIT License
// Copyright (c) 2023 Valerio Cislaghi, Christian Quadri


#ifndef CARLAINETMOBILITY_H_
#define CARLAINETMOBILITY_H_


#include <omnetpp.h>
#include "inet/mobility/base/MobilityBase.h"

using namespace omnetpp;
using namespace std;

/*
 * This class provides additional functionality for handling Carla actors in the INET framework.
 * It defines methods to initialize the position, velocity, and rotation of the actor,
 * update their values for the next step, and retrieve information about the actor's position, velocity, and acceleration.
 * The class also includes methods to get the type of the Carla actor and retrieve the configuration parameters specific to the actor.
 *
 * It is mandatory to set the mobilityType for Carla mobile actors in order to properly configure their mobility behavior.
 */

class CarlaInetMobility : public inet::MobilityBase
{
public:
    // Initialize the position, velocity, and rotation of the actor.
    virtual void preInitialize(const std::string carlaID, const inet::Coord& position, const inet::Coord& velocity, const inet::Quaternion& rotation);

    // Overrides the base class function to perform initialization tasks at a specified stage.
    virtual void initialize(int stage) override;

    // Update the position, velocity, and rotation of the actor for the next step.
    virtual void nextPosition(const inet::Coord& position, const inet::Coord& velocity, const inet::Quaternion& rotation);

    // Returns the current position of the actor.
    virtual const inet::Coord& getCurrentPosition() override;

    // Returns the current velocity of the actor.
    virtual const inet::Coord& getCurrentVelocity() override;

    // Returns the current acceleration of the actor.
    virtual const inet::Coord& getCurrentAcceleration() override;

    // Returns the current angular position of the actor.
    virtual const inet::Quaternion& getCurrentAngularPosition() override;

    // Returns the current angular velocity of the actor.
    virtual const inet::Quaternion& getCurrentAngularVelocity() override;

    // Returns the current angular acceleration of the actor.
    virtual const inet::Quaternion& getCurrentAngularAcceleration() override;

    // Returns the type of the Carla actor.
    string getCarlaActorType() { return carlaActorType; }

    // Returns the Carla id.
    string getCarlaId() { return carla_id; }

    // Returns the configuration parameters of the Carla actor.
    // This method is used by the CarlanetManager to retrieve the configuration for a specific actor that needs to be sent to pycarlanet.
    const cValueMap* getCarlaActorConfiguration() { return carlaActorConfiguration; }

protected:
    // This must be implemented as described in MobilityBase.
    virtual void handleSelfMessage(cMessage* msg) override;

    // Overrides the base class function to set the initial position of the actor.
    virtual void setInitialPosition() override;

    // Updates the Carla actor configuration from parameter values.
    virtual void updateCarlaActorConfigurationFromParam(cValueMap* confMap) {};

    std::string carla_id;
    inet::Coord lastVelocity;
    inet::Quaternion lastAngularVelocity;

    string carlaActorType;

    bool preInitialized = false; // This field is set during dynamic module creation.

protected:
    cValueMap* carlaActorConfiguration;
};

#endif

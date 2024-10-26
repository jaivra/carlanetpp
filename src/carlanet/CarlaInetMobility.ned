// MIT License
// Copyright (c) 2023 Valerio Cislaghi, Christian Quadri



package carlanet;

import inet.mobility.base.MobilityBase;

simple CarlaInetMobility extends MobilityBase {
	parameters:
        @class(CarlaInetMobility);
        @display("i=block/cogwheel");
        @signal[mobilityStateChanged](type=inet::MobilityBase);
        bool initFromDisplayString = default(true); // do not change this to false
        
        string carla_id = default("");
        string carlaActorType = default("car");
        
        object carlaActorConfiguration = default(parseJSON("{}"));
        
}

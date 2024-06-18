// MIT License
// Copyright (c) 2023 Valerio Cislaghi, Christian Quadri

package carlanet;

simple CarlanetManager
{
    parameters:
        @class(CarlanetManager);

        string host = default("localhost");  // pyCARLANeT server hostname
        string protocol = default("tcp");  // pyCARLANeT server protocol
        double simulationTimeStep @unit("s") = default(10ms);
        int port = default(5555);  // pyCARLANeT server port
        //int seed = default(-1); // seed value to set in launch configuration, if missing (-1: current run number)
        int communicationTimeoutms = default(1000);
        //bool autoShutdown = default(true);  // Shutdown module as soon as no more vehicles are in the simulation
		object extraInitParams = default(parseJSON("{}"));
		

		
		
		//Parameters for dynamically generated actors
		// IMPORTANT: all the node types have to use CarlaInetMobility (or module that inherits from it) as mobility module
		string networkActiveModuleType;  		// TODO allow multiple type based on prefixes
		string networkPassiveModuleType = default("inet.node.base.NodeBase");  // TODO allow multiple type based on prefixes
		
		object moduleType;
		object moduleName;
		
		//typeMapping between pycarlanet.ActorType and ModuleType, ModuleName in omnet
		//string typeMapping_Sensor;
        //string typeMapping_Spectator;
        //string typeMapping_Traffic_sign;
        //string typeMapping_Traffic_light;
        //string typeMapping_Vehicle;
        //string typeMapping_Walkers;
		
        @display("i=block/cogwheel");
}


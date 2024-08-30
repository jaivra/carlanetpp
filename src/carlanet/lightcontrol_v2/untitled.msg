//
// TODO generated message class
//

import inet.common.INETDefs;
import inet.common.Units;
import inet.common.packet.chunk.Chunk;
import inet.networklayer.common.L3Address;

namespace inet;

packet Untitled {
    int someField;
    string anotherField;
    double arrayField1[];
    double arrayField2[10];
}

class LightCommandMessage2 extends FieldsChunk{
	//simtime_t creationTime;
	string lightNextState;
}

class LightStatusMessage2 extends FieldsChunk{
	//simtime_t creationTime;
	string lightCurrState;
}
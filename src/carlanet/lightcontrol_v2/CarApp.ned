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

package carlanet.lightcontrol_v2;
import inet.applications.contract.IApp;

//
// TODO auto-generated module
//
simple CarApp like IApp
{
    parameters:
        @class(CarApp);
        @display("i=block/app");
        
        int localPort = default(-1);  // local port (-1: use ephemeral port)
        string localAddress = default("");
        
        volatile int statusMsgLength @unit(B) = default(8kB); // length of messages to generate, in bytes
        
        
        @signal[packetReceived](type=inet::Packet);
	gates:
        input socketIn @labels(UdpControlInfo/up);
        output socketOut @labels(UdpControlInfo/down);
}

/*
 * Noxim - the NoC Simulator
 *
 * (C) 2005-2015 by the University of Catania
 * For the complete list of authors refer to file ../doc/AUTHORS.txt
 * For the license applied to these sources refer to file ../doc/LICENSE.txt
 *
 * This file contains the implementation of the switch reservation table
 */

#include "ReservationTable.h"

ReservationTable::ReservationTable()
{
    rtable.clear();
}


bool ReservationTable::isAvailable(const int port_out)
{
    if (rtable.find(port_out) == rtable.end())
        rtable[port_out].first = NOT_RESERVED;
    
    if (rtable[port_out].first == NOT_VALID) throw NOT_VALID;


    return (rtable[port_out].first == NOT_RESERVED);
}

int ReservationTable::getReservingInput(const int port_out)
{
    if (rtable.find(port_out) == rtable.end())
        return -999;
    
    if (rtable[port_out].first == NOT_VALID) throw NOT_VALID;

    return rtable[port_out].first;
}

void ReservationTable::reserve(const int port_in, const int port_out)
{
    // reservation of reserved/not valid ports is illegal. Correctness
    // should be assured by ReservationTable users
    assert(isAvailable(port_out));
    rtable[port_out] = pair<int,bool>(port_in, false);
}

void ReservationTable::release(const int port_out)
{

    // check wheter there is a port_out entry
    assert(rtable.find(port_out) != rtable.end());

    // there is a valid reservation on port_out
    assert(rtable[port_out].first != NOT_RESERVED);

    rtable[port_out].first = NOT_RESERVED; 
    // Hey! leave transimit info alone! 
    // flit_head release should not be regarded as transmitted
}

bool ReservationTable::has_all_released(const vector<int> output_ports) {
    bool all_released = true;
    for (vector<int>::const_iterator it = output_ports.begin(); it != output_ports.end(); ++it)
    {
        int o = *it;
        if (!isAvailable(o)) {
            all_released = false;
            break;
        }
    }
    return all_released;
}

vector<int> ReservationTable::getOutputPorts(const int port_in)
{
    /*
      Params:
      {int}: input_port
      Return:
      {list}: <output_port>
     */
    assert(port_in >=0 && port_in < DIRECTIONS + 1);
    vector<int> output_ports;

    for (map< int,pair<int,bool> >::iterator i=rtable.begin(); i!=rtable.end(); i++) {
        if (i->second.first == port_in) {
            output_ports.push_back(i->first);
        }
    }
    return output_ports;
}

void ReservationTable::transmitted(const int port_out)
{
    assert(!isAvailable(port_out));
    rtable[port_out].second = true;
}

void ReservationTable::clear_outputs_transmitted(const vector<int> output_ports)
{
    assert(output_ports.size() > 0);
        
    for (vector<int>::const_iterator it = output_ports.begin(); it != output_ports.end(); it++) 
    {
        int o = *it;
        rtable[o].second = false;
    }
}

bool ReservationTable::has_all_transmitted(const vector<int> output_ports)
{
    assert(output_ports.size() > 0);

    bool all_done = true;
        
    for (vector<int>::const_iterator it = output_ports.begin(); it != output_ports.end(); it++) 
    {
        int o = *it;
        if (!rtable[o].second) {
            all_done = false;
            break;
        }
    }
    return all_done;
}

bool ReservationTable::has_transmitted(const int port_out) 
{
    assert(!isAvailable(port_out));
    return rtable[port_out].second;
}

// makes port_out no longer available for reservation/release
void ReservationTable::invalidate(const int port_out)
{
    rtable[port_out].first = NOT_VALID;
}

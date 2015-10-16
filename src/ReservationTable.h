/*
 * Noxim - the NoC Simulator
 *
 * (C) 2005-2010 by the University of Catania
 * For the complete list of authors refer to file ../doc/AUTHORS.txt
 * For the license applied to these sources refer to file ../doc/LICENSE.txt
 *
 * This file contains the declaration of the switch reservation table
 */

#ifndef __NOXIMRESERVATIONTABLE_H__
#define __NOXIMRESERVATIONTABLE_H__

#include <cassert>
#include "DataStructs.h"
#include "Utils.h"

using namespace std;

class ReservationTable {
public:

    ReservationTable();

    inline string name() const {return "ReservationTable";};


    // check if port_out is reservable
    bool isAvailable(const int port_out);

    // Connects port_in with port_out. Asserts if port_out is reserved
    void reserve(const int port_in, const int port_out);

    // Record port_out has been transmitted
    void transmitted(const int port_out);

    // Clear port_in's transmittion info
    void clear_outputs_transmitted(const vector<int> output_ports);

    // Check if (port_out) has been all transmitted
    bool has_transmitted(const int port_out);

    // Check if all output_ports have been transmitted
    bool has_all_transmitted(const vector<int> output_ports);


    // Releases port_out connection. 
    // Asserts if port_out is not reserved or not valid
    void release(const int port_out);

    // Check if all output_ports are released
    bool has_all_released(const vector<int> output_ports);

    // Returns the output_port list connected to port_in.
    vector<int> getOutputPorts(const int port_in);

    // Makes output port no longer available for reservation/release
    void invalidate(const int port_out);


private:
    /* Reservation Table: 
       key: output_port
       value: (input_port, has_transmitted)
     */
    map< int, pair<int,bool> > rtable;
};

#endif

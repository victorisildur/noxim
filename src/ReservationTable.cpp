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
  if (rtable.find(port_out)==rtable.end())
    rtable[port_out] = NOT_RESERVED;
    
  if (rtable[port_out]==NOT_VALID) throw NOT_VALID;


  return (rtable[port_out]==NOT_RESERVED);
}

void ReservationTable::reserve(const int port_in, const int port_out)
{
    // reservation of reserved/not valid ports is illegal. Correctness
    // should be assured by ReservationTable users
    assert(isAvailable(port_out));

    rtable[port_out] = port_in;
}

void ReservationTable::release(const int port_out)
{

    // check wheter there is a port_out entry
    assert(rtable.find(port_out)!=rtable.end());

    // there is a valid reservation on port_out
    assert(rtable[port_out]!=NOT_RESERVED);

    rtable[port_out] = NOT_RESERVED;
}

vector<int> ReservationTable::getOutputPort(const int port_in)
{
  vector<int> output_ports;

  for (map<int,int>::iterator i=rtable.begin(); i!=rtable.end(); i++) {
	if (i->second == port_in) {
      output_ports.push_back(i->first);
    }
  }
  if (output_ports.empty()) {
    // semantic: port_in currently doesn't reserve any out port
    output_ports.push_back(NOT_RESERVED);
  }
  return output_ports;
}

// makes port_out no longer available for reservation/release
void ReservationTable::invalidate(const int port_out)
{
  rtable[port_out] = NOT_VALID;
}

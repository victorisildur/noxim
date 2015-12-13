/*
 * Noxim - the NoC Simulator
 *
 * (C) 2005-2010 by the University of Catania
 * For the complete list of authors refer to file ../doc/AUTHORS.txt
 * For the license applied to these sources refer to file ../doc/LICENSE.txt
 *
 * This file contains the declaration of the buffer
 */

#ifndef __NOXIMBACKUPUNIT_H__
#define __NOXIMBACKUPUNIT_H__

#include <cassert>
#include <queue>
#include "DataStructs.h"
using namespace std;

class BackupUnit {

  public:

    BackupUnit();

    virtual ~ BackupUnit() {
    } 

    void setMaxBackupSize(const unsigned int bms);	// Set backup max size (in flits)

    unsigned int getMaxBackupSize() const;	// Get backup max size (in flits)

    Flit pop();		// Pop a flit

    Flit front() const;	// Return a copy of the first flit in the backup

    unsigned int size() const;

    void startBackUp();

    void endBackUp();

    void backUp(const Flit &flit);

    void setExpectPorts(const vector<int> expect_ports);

    bool isEmpty() const;	// Returns true if backup is empty    

    bool isFull() const;	// Returns true if backup is empty    

    bool isBackingUp() const;	// Returns true if backup unit is backing up

    vector<int> getExpectPorts() const;

    inline string name() { return "BackupUnit";};

    void deleteExpectPort(int i);

    void addExpectPort(int i);

    void clear();

    void print();

  private:

    unsigned int max_buffer_size;

    queue < Flit > buffer;

    vector<int> _expect_outputs;

    bool _is_backing_up;

    int _last_flit_seqno;

    void push(const Flit & flit);	// Push a flit. Calls Drop method if buffer is full

    virtual void drop(const Flit & flit) const;	// Called by Push() when buffer is full

    virtual void empty() const;	// Called by Pop() when buffer is empty

};

#endif

/*
 * Noxim - the NoC Simulator
 *
 * (C) 2005-2010 by the University of Catania
 * For the complete list of authors refer to file ../doc/AUTHORS.txt
 * For the license applied to these sources refer to file ../doc/LICENSE.txt
 *
 * This file contains the implementation of the buffer
 */

#include "BackupUnit.h"
#include "Utils.h"

BackupUnit::BackupUnit()
{
    setMaxBackupSize(GlobalParams::max_packet_size);
    _is_backing_up = false;
    _last_flit_seqno = -1;
}


void BackupUnit::setMaxBackupSize(const unsigned int bms)
{
    assert(bms > 0);

    max_buffer_size = bms;
}

unsigned int BackupUnit::getMaxBackupSize() const
{
    return max_buffer_size;
}

bool BackupUnit::isFull() const
{
    return buffer.size() >= max_buffer_size;
}

bool BackupUnit::isEmpty() const
{
    return buffer.size() == 0;
}

void BackupUnit::drop(const Flit & flit) const
{
    assert(false);
}

void BackupUnit::empty() const
{
    assert(false);
}

void BackupUnit::push(const Flit & flit)
{
    if (isFull())
        drop(flit);
    else
        buffer.push(flit);
}

Flit BackupUnit::pop()
{
    Flit f;

    if (isEmpty()) {
        empty();
    } else {
        f = buffer.front();
        buffer.pop();
        buffer.push(f); // cycle queue!
    }

    return f;
}

Flit BackupUnit::front() const
{
    Flit f;

    if (isEmpty())
        empty();
    else
        f = buffer.front();

    return f;
}

unsigned int BackupUnit::size() const
{
    return buffer.size();
}

void BackupUnit::startBackUp()
{
    _is_backing_up = true;
}

void BackupUnit::endBackUp()
{
    _is_backing_up = false;
}

bool BackupUnit::isBackingUp() const
{
    return _is_backing_up;
}

void BackupUnit::backUp(const Flit &flit)
{
    assert(_is_backing_up);

    if (flit.sequence_no == _last_flit_seqno)
        return;

    // make sure queue is not full before push
    if (isFull()) {
        LOG_EZ << "backup unit is full, bu size:" << getMaxBackupSize() 
               << " last flit:" << flit << endl;
        while (!buffer.empty()) {
            LOG_EZ << "backup unit content: " << buffer.front() << endl;
            buffer.pop();
        }
        for (vector<int>::iterator it = _expect_outputs.begin(); it != _expect_outputs.end(); ++it) {
            LOG_EZ << "backup unit expect: " << *it << endl;
        }
        assert(false);
    }

    push(flit);

    _last_flit_seqno = flit.sequence_no;
}

void BackupUnit::setExpectPorts(const vector<int> expect_ports)
{
    assert(expect_ports.size() > 0);
    if (_expect_outputs.size() > 0)
        _expect_outputs.clear();
    for (vector<int>::const_iterator it = expect_ports.begin(); it != expect_ports.end(); ++it)
    {
        _expect_outputs.push_back(*it);
    }
}


vector<int> BackupUnit::getExpectPorts() const
{
    return _expect_outputs;
}

void BackupUnit::deleteExpectPort(int i) 
{
    bool found = false;
    for (vector<int>::iterator it = _expect_outputs.begin(); it != _expect_outputs.end(); ++it)
    {
        if (i == *it) {
            _expect_outputs.erase(it--);
            found = true;
        }
    }
    assert(found);
}

void BackupUnit::addExpectPort(int i)
{
    bool found = false;
    for (vector<int>::iterator it = _expect_outputs.begin(); it != _expect_outputs.end(); ++it)
    {
        if (i == *it) {
            found = true;
        }
    }
    assert(!found);
    _expect_outputs.push_back(i);
}

void BackupUnit::clear()
{
    // clear buffer
    while (!buffer.empty()) {
        buffer.pop();
    }
    // clear expect outputs
    if (!_expect_outputs.empty()) {
        _expect_outputs.clear();
    }
    _last_flit_seqno = -1;
    assert(_expect_outputs.empty());
}

void BackupUnit::print()
{
    cout << "  is backing up: " << isBackingUp() << endl;
    cout << "  is expecting: ";
    for (vector<int>::iterator it = _expect_outputs.begin(); it != _expect_outputs.end(); ++it)
    {
        cout << *it << " ";
    }
    cout << endl;
    cout << "  content: ";
    queue<Flit> m = buffer;
    while (!(m.empty()))
    {
        Flit f = m.front();
        m.pop();
        cout << f << " | ";
    }
    cout << endl;

}

/*
 * Noxim - the NoC Simulator
 *
 * (C) 2005-2015 by the University of Catania
 * For the complete list of authors refer to file ../doc/AUTHORS.txt
 * For the license applied to these sources refer to file ../doc/LICENSE.txt
 *
 * This file contains the implementation of the router
 */

#include "Router.h"

void Router::rxProcess()
{
    if (reset.read()) {
        // Clear outputs and indexes of receiving protocol
        for (int i = 0; i < DIRECTIONS + 2; i++) {
            ack_rx[i].write(0);
            current_level_rx[i] = 0;
        }
        routed_flits = 0;
        local_drained = 0;
    } else {
        // For each channel decide if a new flit can be accepted
        //
        // This process simply sees a flow of incoming flits. All arbitration
        // and wormhole related issues are addressed in the txProcess()

        for (int i = 0; i < DIRECTIONS + 2; i++) {
            // To accept a new flit, the following conditions must match:
            //
            // 1) there is an incoming request
            // 2) there is a free slot in the input buffer of direction i

            if ((req_rx[i].read() == 1 - current_level_rx[i])
                && !buffer[i].IsFull()) {
                Flit received_flit = flit_rx[i].read();

                // if a new flit is injected from local PE
                if (received_flit.src_id == local_id)
                {
                    // set broadcast routine for broadcast flits
                    if (received_flit.dst_id == -1)
                    {
                        if (received_flit.flit_type == FLIT_TYPE_HEAD) 
                        {
                            // when flit_head, check router congestion
                            received_flit.broadcast_routine = BROADCAST_TREE;
                            if (isInjectCongested()) {
                                received_flit.broadcast_routine = BROADCAST_PATH;
                                LOG_L << " congested, flit: " << received_flit << endl;
                            } else {
                                LOG_L << " idle, flit: " << received_flit << endl;
                            }
                            _broadcast_routine = received_flit.broadcast_routine;
                        }
                        else
                        {
                            // all following flit should have same header with head_flit
                            received_flit.broadcast_routine = _broadcast_routine;
                        }
                    }
                    power.networkInterface();
                }

                // Store the incoming flit in the circular buffer
                buffer[i].Push(received_flit);

                power.bufferRouterPush();

                // Negate the old value for Alternating Bit Protocol (ABP)
                current_level_rx[i] = 1 - current_level_rx[i];


            }
            ack_rx[i].write(current_level_rx[i]);
        }
    }
}



void Router::txProcess()
{
    if (reset.read()) 
    {
        // Clear outputs and indexes of transmitting protocol
        for (int i = 0; i < DIRECTIONS + 2; i++) 
        {
            req_tx[i].write(0);
            current_level_tx[i] = 0;
        }
    } 
    else 
    {

        for (int j = 0; j < DIRECTIONS + 2; j++) {
            int i = (start_from_port + j) % (DIRECTIONS + 2);
            // 1st phase: Reservation
            reserveForInput(i);
            // 2nd phase: Backup
            backupInput(i);
        }

        start_from_port++;

        // 3rd phase: Forwarding
        for (int i = 0; i < DIRECTIONS + 2; i++) {
            forwardInput(i);
        }
    }
}

void Router::reserveForInput(int i)
{
    /* Reserve by BACKUP's expect ports, 
     * when backup is not empty and is expecting ports and is not backing up
     */
    if (!backup_unit[i].isEmpty() && 
        backup_unit[i].getExpectPorts().size() > 0 &&
        !backup_unit[i].isBackingUp() ) 
    {
        vector<int> backup_expecting_ports = backup_unit[i].getExpectPorts();
        int o = backup_expecting_ports[0];
        if (reservation_table.isChannelAvailable(i,o)) 
        {
            LOG_M << " reserve ok " << i << "->" << o << " for backup " << endl;
            reservation_table.reserve(i, o);
            backup_unit[i].deleteExpectPort(o);
        } 
        else
        {
            LOG_M << " cannot reserve " << i << "->" << o << " for backup " << endl;
            LOG_M << o << " is occupied by " << reservation_table.getInputOccupyingOutput(o) << endl;
        }
        return;
    }

    /* Reserve by BUFFER's front, 
     * only after BU has transmit done
     */
    //if (!buffer[i].deadlockFree())
    if (false)
    {
        LOG << " deadlock on buffer input " << i << endl;
        buffer[i].Print(" deadlock ");
        for (int j=0; j<DIRECTIONS+2; j++) {
            LOG << "          on buffer input " << j << endl;
            buffer[j].Print("          ");
        }
        LOG << "reservation table:" << endl;
        reservation_table.print();
        for (int j=0; j<DIRECTIONS+2; j++) {
            LOG << " on backup_unit " << j << endl;
            backup_unit[j].print();
        }
        assert(false);
    }

    if (buffer[i].IsEmpty()) 
        return;

    Flit flit = buffer[i].Front();
    power.bufferRouterFront();

    if (flit.flit_type == FLIT_TYPE_HEAD) 
    {
        // prepare data for routing
        RouteData route_data;
        route_data.current_id = local_id;
        route_data.src_id = flit.src_id;
        route_data.dst_id = flit.dst_id;
        route_data.dir_in = i;
        route_data.broadcast_routine = flit.broadcast_routine;
        route_data.path_dir = flit.path_dir;

        vector<int> output_ports = route(route_data);
        int o = output_ports[0];

        // try to reserve i->o
        if (reservation_table.isChannelAvailable(i,o)) {
            LOG_M << "reserve ok " << i << "->" << o << " for flit " << flit << endl;
            reservation_table.reserve(i, o);
            // reserve for buffer[i] ok
            // but there are not reserved outputs, tell backup start to backup
            if (output_ports.size() > 1)
            {
                LOG_M << "start backing up " << i << endl;
                backup_unit[i].startBackUp();
                vector<int> expect_ports;
                for (vector<int>::iterator it = output_ports.begin(); it != output_ports.end(); ++it) 
                {
                    if (*it != o)
                        expect_ports.push_back(*it);
                }
                backup_unit[i].setExpectPorts(expect_ports);
            }

        } else {
            LOG_M << "cannot reserve " << i << "->" << o << " for flit " << flit << endl;
            LOG_M << o << " is occupied by " << reservation_table.getInputOccupyingOutput(o) << endl;
        }

    }
}

void Router::backupInput(int i)
{
    if (!backup_unit[i].isBackingUp()) {
        return;
    }
    if (buffer[i].IsEmpty())
        return;
    
    Flit flit = buffer[i].Front();

    LOG_M << "backing up " << i << " , flit: " << flit << endl;

    backup_unit[i].backUp(flit);
    
}

void Router::forwardInput(int i)
{
    Flit flit;
    bool isFromBackup = false;

    if (backup_unit[i].isEmpty() || backup_unit[i].isBackingUp()) {
        // power contribution already computed in 1st phase
        if (buffer[i].IsEmpty()) 
            return;
        flit = buffer[i].Front();
    } else {
        flit = backup_unit[i].front();
        isFromBackup = true;
    }
    
    int o = reservation_table.getOutputPort(i);
    if (o == NOT_RESERVED) 
        return;

    if (current_level_tx[o] == ack_tx[o].read()) 
    {
        LOG_M << "forward ok " << i << "->" << o << ", flit: " << flit 
              << " , if from backup:" << isFromBackup << endl;

        flit_tx[o].write(flit);
        if (o == DIRECTION_HUB) {
            power.r2hLink();
            LOG << "Forwarding to HUB " << flit << endl;
        } else {
            power.r2rLink();
        }

        power.crossBar();

        current_level_tx[o] = 1 - current_level_tx[o];
        req_tx[o].write(current_level_tx[o]);
        if (!isFromBackup) {
            buffer[i].Pop();
            power.bufferRouterPop();
        } else {
            backup_unit[i].pop();
            // TODO: better power model for backup
            power.bufferRouterPop();
        }
        
        // if flit has been consumed
        if (flit.dst_id == local_id)
            power.networkInterface();

        if (flit.flit_type == FLIT_TYPE_TAIL) {
            LOG_M << " release " << i << "->" << o << ", flit: " << flit << endl;
            reservation_table.release(o);
            // if forward from input and backup_unit is backing up. End backup!
            if (backup_unit[i].isBackingUp()) {
                assert(!isFromBackup);
                backup_unit[i].endBackUp();
                LOG_M << "end backup " << i << endl;
            }
            // if forward from backup, and backup_unit is expecting no ports. Clear backup!
            if (isFromBackup && backup_unit[i].getExpectPorts().size() == 0) {
                backup_unit[i].clear();
            }
        }
        // Update stats
        if (o == DIRECTION_LOCAL) 
        {
            LOG_M << i << "->" << o << " consumed flit " << flit << endl;

            stats.receivedFlit(sc_time_stamp().to_double() / GlobalParams::clock_period_ps, flit);
            if (GlobalParams:: max_volume_to_be_drained) 
            {
                if (drained_volume >= GlobalParams:: max_volume_to_be_drained)
                    sc_stop();
                else 
                {
                    drained_volume++;
                    local_drained++;
                }
            }
        } 
        else if (i != DIRECTION_LOCAL) 
        {
            // Increment routed flits counter
            routed_flits++;
        }
    }
    else
    {
        LOG_M << " cannot forward " << i << "->" << o << ", flit: " << flit
              << " , if from backup:" << isFromBackup << endl;
        if (flit.flit_type == FLIT_TYPE_HEAD) {
            reservation_table.release(o);
            LOG_M << " release " << i << "->" << o << ", flit: " << flit << endl;   
            if (isFromBackup)
                backup_unit[i].addExpectPort(o);
        }
    }
}

NoP_data Router::getCurrentNoPData()
{
    NoP_data NoP_data;

    for (int j = 0; j < DIRECTIONS; j++) {
        try {
            NoP_data.channel_status_neighbor[j].free_slots = free_slots_neighbor[j].read();
            NoP_data.channel_status_neighbor[j].available = (reservation_table.isAvailable(j));
        }
        catch (int e)
        {
            if (e!=NOT_VALID) assert(false);
            // Nothing to do if an NOT_VALID direction is caught
        };
    }

    NoP_data.sender_id = local_id;

    return NoP_data;
}

void Router::perCycleUpdate()
{
    if (reset.read()) {
        for (int i = 0; i < DIRECTIONS + 1; i++)
            free_slots[i].write(buffer[i].GetMaxBufferSize());
    } else {
        selectionStrategy->perCycleUpdate(this);

        power.leakageRouter();
        for (int i = 0; i < DIRECTIONS + 1; i++)
        {
            power.leakageBufferRouter();
            power.leakageLinkRouter2Router();
        }

        power.leakageLinkRouter2Hub();
    }
}

vector < int > Router::routingFunction(const RouteData & route_data)
{
    if (GlobalParams::use_winoc)
    {
        if (hasRadioHub(local_id) &&
            hasRadioHub(route_data.dst_id) &&
            !sameRadioHub(local_id,route_data.dst_id)
            )
        {
            LOG << "Setting direction HUB to reach destination node " << route_data.dst_id << endl;

            vector<int> dirv;
            dirv.push_back(DIRECTION_HUB);
            return dirv;
        }
    }
    LOG_M << "Wired routing for dst = " << route_data.dst_id << endl;

    return routingAlgorithm->route(this, route_data);
}

vector<int> Router::route(const RouteData & route_data)
{
    vector<int> out_ports;
    if (route_data.dst_id == local_id || (route_data.dst_id == -1 && route_data.src_id != local_id)) {
        /* unicast consume, broadcast consume */
        out_ports.push_back(DIRECTION_LOCAL);
    }
    if (route_data.dst_id != local_id || route_data.dst_id == -1) {
        /* unicast on-the-way, broadcast */
        power.routing();
        power.selection();
        vector<int> others = routingFunction(route_data);
        for(vector<int>::iterator it = others.begin(); it != others.end(); ++it) {
            out_ports.push_back(*it);
        }
    }
    assert(out_ports.size() > 0);
    return out_ports;
}

void Router::NoP_report() const
{
    NoP_data NoP_tmp;
    LOG << "NoP report: " << endl;

    for (int i = 0; i < DIRECTIONS; i++) {
        NoP_tmp = NoP_data_in[i].read();
        if (NoP_tmp.sender_id != NOT_VALID)
            cout << NoP_tmp;
    }
}

//---------------------------------------------------------------------------

int Router::NoPScore(const NoP_data & nop_data,
                     const vector < int >&nop_channels) const
{
    int score = 0;

    for (unsigned int i = 0; i < nop_channels.size(); i++) {
        int available;

        if (nop_data.channel_status_neighbor[nop_channels[i]].available)
            available = 1;
        else
            available = 0;

        int free_slots =
            nop_data.channel_status_neighbor[nop_channels[i]].free_slots;

        score += available * free_slots;
    }

    return score;
}

int Router::selectionFunction(const vector < int >&directions,
                              const RouteData & route_data)
{
    // not so elegant but fast escape ;)
    if (directions.size() == 1)
        return directions[0];

    return selectionStrategy->apply(this, directions, route_data);
}

void Router::configure(const int _id,
                       const double _warm_up_time,
                       const unsigned int _max_buffer_size,
                       GlobalRoutingTable & grt)
{
    local_id = _id;
    stats.configure(_id, _warm_up_time);

    start_from_port = DIRECTION_LOCAL;

    if (grt.isValid())
        routing_table.configure(grt, _id);

    for (int i = 0; i < DIRECTIONS + 2; i++)
        buffer[i].SetMaxBufferSize(_max_buffer_size);

    int row = _id / GlobalParams::mesh_dim_x;
    int col = _id % GlobalParams::mesh_dim_x;
    if (row == 0)
        buffer[DIRECTION_NORTH].Disable();
    if (row == GlobalParams::mesh_dim_y-1)
        buffer[DIRECTION_SOUTH].Disable();
    if (col == 0)
        buffer[DIRECTION_WEST].Disable();
    if (col == GlobalParams::mesh_dim_x-1)
        buffer[DIRECTION_EAST].Disable();

}

unsigned long Router::getRoutedFlits()
{
    return routed_flits;
}

unsigned int Router::getFlitsCount()
{
    unsigned count = 0;

    for (int i = 0; i < DIRECTIONS + 2; i++)
        count += buffer[i].Size();

    return count;
}


int Router::reflexDirection(int direction) const
{
    if (direction == DIRECTION_NORTH)
        return DIRECTION_SOUTH;
    if (direction == DIRECTION_EAST)
        return DIRECTION_WEST;
    if (direction == DIRECTION_WEST)
        return DIRECTION_EAST;
    if (direction == DIRECTION_SOUTH)
        return DIRECTION_NORTH;

    // you shouldn't be here
    assert(false);
    return NOT_VALID;
}

int Router::getNeighborId(int _id, int direction) const
{
    Coord my_coord = id2Coord(_id);

    switch (direction) {
    case DIRECTION_NORTH:
        if (my_coord.y == 0)
            return NOT_VALID;
        my_coord.y--;
        break;
    case DIRECTION_SOUTH:
        if (my_coord.y == GlobalParams::mesh_dim_y - 1)
            return NOT_VALID;
        my_coord.y++;
        break;
    case DIRECTION_EAST:
        if (my_coord.x == GlobalParams::mesh_dim_x - 1)
            return NOT_VALID;
        my_coord.x++;
        break;
    case DIRECTION_WEST:
        if (my_coord.x == 0)
            return NOT_VALID;
        my_coord.x--;
        break;
    default:
        LOG << "Direction not valid : " << direction;
        assert(false);
    }

    int neighbor_id = coord2Id(my_coord);

    return neighbor_id;
}

bool Router::inCongestion()
{
    for (int i = 0; i < DIRECTIONS; i++) {

        if (free_slots_neighbor[i]==NOT_VALID) continue;

        int flits = GlobalParams::buffer_depth - free_slots_neighbor[i];
        if (flits > (int) (GlobalParams::buffer_depth * GlobalParams::dyad_threshold))
            return true;
    }

    return false;
}

bool Router::isInjectCongested()
{
    int free_buf = buffer[DIRECTION_LOCAL].getCurrentFreeSlots();
    int total_buf = buffer[DIRECTION_LOCAL].GetMaxBufferSize();
    
    float free_rate = free_buf / total_buf;
    if (free_rate < 1 - GlobalParams::inject_congestion_threshold) 
        return true;
    else 
        return false;
}

void Router::ShowBuffersStats(std::ostream & out)
{
    for (int i=0; i<DIRECTIONS+2; i++)
        buffer[i].ShowStats(out);
}

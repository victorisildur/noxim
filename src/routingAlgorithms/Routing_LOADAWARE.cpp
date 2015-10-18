#include "Routing_LOADAWARE.h"

RoutingAlgorithmsRegister Routing_LOADAWARE::routingAlgorithmsRegister("LOADAWARE", getInstance());

Routing_LOADAWARE * Routing_LOADAWARE::routing_LOADAWARE = 0;

Routing_LOADAWARE * Routing_LOADAWARE::getInstance() {
	if ( routing_LOADAWARE == 0 )
		routing_LOADAWARE = new Routing_LOADAWARE();
    
	return routing_LOADAWARE;
}

vector<int> Routing_LOADAWARE::route(Router * router, const RouteData & routeData)
{
    Coord current = id2Coord(routeData.current_id);
    Coord destination = id2Coord(routeData.dst_id);
    Coord source = id2Coord(routeData.src_id);

    vector <int> directions;
    if (routeData.dst_id == -1) // broadcast
    {
        if (routeData.broadcast_routine == BROADCAST_PATH)
        {
            /* path-based */
            int curr_label = coord2Label(current);
            int src_label = coord2Label(source);
            Coord next;
            if (curr_label >= src_label && 
                curr_label < GlobalParams::mesh_dim_x * GlobalParams::mesh_dim_y - 1) 
            {
                next = label2Coord(curr_label + 1);
                directions.push_back(calcDir(next,current));
            }
            if (curr_label <= src_label && 
                curr_label > 0) 
            {
                next = label2Coord(curr_label - 1);
                directions.push_back(calcDir(next,current));
            }
        }
        else {
            /* tree-based */
            if (current.y >= source.y && current.y < GlobalParams::mesh_dim_y - 1)
                directions.push_back(DIRECTION_SOUTH);
            if (current.y <= source.y && current.y > 0)
                directions.push_back(DIRECTION_NORTH);
            if (current.y == source.y) 
            {
                if (current.x >= source.x && current.x < GlobalParams::mesh_dim_x -1) {
                    directions.push_back(DIRECTION_EAST);
                }
                if (current.x <= source.x && current.x > 0) {
                    directions.push_back(DIRECTION_WEST);
                }
            }
        }
    } 
    else 
    {
        /* unicast */
        if (destination.x > current.x)
            directions.push_back(DIRECTION_EAST);
        else if (destination.x < current.x)
            directions.push_back(DIRECTION_WEST);
        else if (destination.y > current.y)
            directions.push_back(DIRECTION_SOUTH);
        else
            directions.push_back(DIRECTION_NORTH);
    }
    return directions;
}

int Routing_LOADAWARE::coord2Label(const Coord coord)
{
    int x = coord.x;
    int y = coord.y;

    if(y%2 == 0)
        return (y * GlobalParams::mesh_dim_x + x);
    else
        return ( (y+1) * GlobalParams::mesh_dim_x - x -1 );
}

Coord Routing_LOADAWARE::label2Coord(const int label)
{
    Coord coord;
    if( (label / GlobalParams::mesh_dim_x) % 2 == 0) {
        coord.y = label / GlobalParams::mesh_dim_x;
        coord.x = label - coord.y * GlobalParams::mesh_dim_x;
    }
    else {
        coord.y = label / GlobalParams::mesh_dim_x;
        coord.x = (coord.y + 1) * GlobalParams::mesh_dim_x - label - 1;
    }
    return coord;
}

int Routing_LOADAWARE::calcDir(const Coord next, const Coord curr)
{
    if(next.y > curr.y)
        return DIRECTION_SOUTH;
    if(next.y < curr.y)
        return DIRECTION_NORTH;
    if(next.x > curr.x)
        return DIRECTION_EAST;
    return DIRECTION_WEST;
}

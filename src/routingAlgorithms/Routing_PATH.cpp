#include "Routing_PATH.h"

RoutingAlgorithmsRegister Routing_PATH::routingAlgorithmsRegister("PATH", getInstance());

Routing_PATH * Routing_PATH::routing_PATH = 0;

Routing_PATH * Routing_PATH::getInstance() {
	if ( routing_PATH == 0 )
		routing_PATH = new Routing_PATH();
    
	return routing_PATH;
}

vector<int> Routing_PATH::route(Router * router, const RouteData & routeData)
{
    Coord current = id2Coord(routeData.current_id);
    Coord destination = id2Coord(routeData.dst_id);
    Coord source = id2Coord(routeData.src_id);

    vector <int> directions;
    if (routeData.dst_id == -1) 
    {
        /* broadcast, path-based */
        int curr_label = coord2Label(current);
        int src_label = coord2Label(source);

        if (curr_label >= src_label && 
            curr_label < GlobalParams::mesh_dim_x * GlobalParams::mesh_dim_y - 1) 
        {
            Coord next1;
            next1 = label2Coord(curr_label + 1);
            directions.push_back(calcDir(next1,current));
        }
        if (curr_label <= src_label && 
            curr_label > 0) 
        {
            Coord next2;
            next2 = label2Coord(curr_label - 1);
            directions.push_back(calcDir(next2,current));
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

int Routing_PATH::coord2Label(const Coord coord)
{
    int x = coord.x;
    int y = coord.y;
    if(y%2 == 0)
        return (y * GlobalParams::mesh_dim_x + x);
    else
        return ( (y+1) * GlobalParams::mesh_dim_x - x -1 );
}

Coord Routing_PATH::label2Coord(const int label)
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

int Routing_PATH::calcDir(const Coord next, const Coord curr)
{
    if(next.y > curr.y)
        return DIRECTION_SOUTH;
    if(next.y < curr.y)
        return DIRECTION_NORTH;
    if(next.x > curr.x)
        return DIRECTION_EAST;
    return DIRECTION_WEST;
}

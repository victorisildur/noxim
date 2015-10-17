#include "Routing_XY.h"

RoutingAlgorithmsRegister Routing_XY::routingAlgorithmsRegister("XY", getInstance());

Routing_XY * Routing_XY::routing_XY = 0;

Routing_XY * Routing_XY::getInstance() {
	if ( routing_XY == 0 )
		routing_XY = new Routing_XY();
    
	return routing_XY;
}

vector<int> Routing_XY::route(Router * router, const RouteData & routeData)
{
    Coord current = id2Coord(routeData.current_id);
    Coord destination = id2Coord(routeData.dst_id);
    Coord source = id2Coord(routeData.src_id);

    vector <int> directions;
    if (routeData.dst_id == -1) 
    {
        /* broadcast, tree-based */
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


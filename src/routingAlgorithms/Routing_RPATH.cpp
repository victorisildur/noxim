#include "Routing_RPATH.h"

RoutingAlgorithmsRegister Routing_RPATH::routingAlgorithmsRegister("RPATH", getInstance());

Routing_RPATH * Routing_RPATH::routing_RPATH = 0;

Routing_RPATH * Routing_RPATH::getInstance() {
	if ( routing_RPATH == 0 )
		routing_RPATH = new Routing_RPATH();
    
	return routing_RPATH;
}

vector<int> Routing_RPATH::route(Router * router, const RouteData & routeData)
{
    Coord current = id2Coord(routeData.current_id);
    Coord destination = id2Coord(routeData.dst_id);
    Coord source = id2Coord(routeData.src_id);

    vector <int> directions;
    if (routeData.dst_id == -1) 
    {
        /* broadcast, path-based */
        int curr_label, src_label;
        if (routeData.path_dir == PATH_HORIZONTAL) {
            curr_label = coord2Hlabel(current);
            src_label = coord2Hlabel(source);
        } else {
            curr_label = coord2Vlabel(current);
            src_label = coord2Vlabel(source);
        }

        if (curr_label >= src_label && 
            curr_label < GlobalParams::mesh_dim_x * GlobalParams::mesh_dim_y - 1) 
        {
            Coord next1;
            if (routeData.path_dir == PATH_HORIZONTAL)
                next1 = hlabel2Coord(curr_label + 1);
            else
                next1 = vlabel2Coord(curr_label + 1);
            directions.push_back(calcDir(next1,current));
            if (GlobalParams::verbose_mode >= VERBOSE_HIGH)
                LOG_EZ << "nx,ny: " << next1.x << "," << next1.y << endl;
        }

        if (curr_label <= src_label && 
            curr_label > 0) 
        {
            Coord next2;
            if (routeData.path_dir == PATH_HORIZONTAL)
                next2 = hlabel2Coord(curr_label - 1);
            else
                next2 = vlabel2Coord(curr_label - 1);
            directions.push_back(calcDir(next2,current));
            if (GlobalParams::verbose_mode >= VERBOSE_HIGH)
                LOG_EZ << "nx,ny: " << next2.x << "," << next2.y << endl;
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

int Routing_RPATH::coord2Hlabel(const Coord coord)
{
    int x = coord.x;
    int y = coord.y;
    if(y%2 == 0)
        return (y * GlobalParams::mesh_dim_x + x);
    else
        return ( (y+1) * GlobalParams::mesh_dim_x - x -1 );
}

Coord Routing_RPATH::hlabel2Coord(const int label)
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

int Routing_RPATH::coord2Vlabel(const Coord coord)
{
    int x = coord.x;
    int y = coord.y;
    if(x%2 == 0)
        return (x * GlobalParams::mesh_dim_y + y);
    else
        return ( (x+1) * GlobalParams::mesh_dim_y - y -1 );
}

Coord Routing_RPATH::vlabel2Coord(const int label)
{
    Coord coord;
    if( (label / GlobalParams::mesh_dim_y) % 2 == 0) {
        coord.x = label / GlobalParams::mesh_dim_y;
        coord.y = label - coord.x * GlobalParams::mesh_dim_y;
    }
    else {
        coord.x = label / GlobalParams::mesh_dim_y;
        coord.y = (coord.x + 1) * GlobalParams::mesh_dim_y - label - 1;
    }
    return coord;
}

int Routing_RPATH::calcDir(const Coord next, const Coord curr)
{
    if(next.y > curr.y)
        return DIRECTION_SOUTH;
    if(next.y < curr.y)
        return DIRECTION_NORTH;
    if(next.x > curr.x)
        return DIRECTION_EAST;
    return DIRECTION_WEST;
}

#ifndef __NOXIMROUTING_PATH_H__
#define __NOXIMROUTING_PATH_H__

#include "RoutingAlgorithm.h"
#include "RoutingAlgorithms.h"
#include "../Router.h"

using namespace std;

class Routing_PATH : RoutingAlgorithm {
public:
    vector<int> route(Router * router, const RouteData & routeData);

    static Routing_PATH * getInstance();

private:
    Routing_PATH(){};
    ~Routing_PATH(){};

    static Routing_PATH * routing_PATH;
    static RoutingAlgorithmsRegister routingAlgorithmsRegister;

    int coord2Label(const Coord coord);
    Coord label2Coord(const int label);
    int calcDir(const Coord next, const Coord curr);
};

#endif

#ifndef MAIN_HEADER
#define MAIN_HEADER

#include <jmutex.h>
using namespace jthread; // JThread 1.3 support

#define PI 3.14159

#define FOV_ANGLE (PI/2.5)

#include "common_irrlicht.h"
// Change to struct settings or something
extern s16 g_viewing_range_nodes;
extern JMutex g_viewing_range_nodes_mutex;
#include <iostream>
#include <fstream>
#include <json/json.h>

//// -----File stream-----
//extern std::ostream mapClient;
//extern std::ostream mapServer;
// Debug streams
extern std::ostream dout_con;
extern std::ostream dout_client;
extern std::ostream dout_server;
extern std::ostream dout_map;
#endif


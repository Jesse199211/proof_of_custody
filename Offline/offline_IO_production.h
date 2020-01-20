/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2019, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#ifndef _Offline_Input_Production
#define _Offline_Input_Production

#include <list>
using namespace std;

#include "LSSS/Open_Protocol.h"
#include "LSSS/PRSS.h"
#include "offline_data.h"

void make_IO_data_fake(Player &P, unsigned int player_num, list<Share> &a,
                       list<gfp> &opened);

void make_IO_data_non_Full(Player &P, PRSS &prss, unsigned int player_num,
                           list<Share> &a, list<gfp> &opened,
                           Open_Protocol &OP);

void make_IO_data(Player &P, int fake_sacrifice, PRSS &prss,
                  unsigned int player_num, list<Share> &a, list<gfp> &opened,
                  offline_control_data &OCD, Open_Protocol &OP,
                  unsigned int num_online);
#endif

/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2019, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#ifndef _RunTime
#define _RunTime

/* This file holds the main SCALE runtime in a
 * single function call, to allow programmatic
 * setup of SCALE outside of using Player.x
 *
 */

#include "Offline/offline_data.h"
#include "Online/Machine.h"
#include "System/SystemData.h"

#include <openssl/ssl.h>

void Run_Scale(unsigned int my_number, unsigned int no_online_threads, const vector<gfp> &MacK,
               SSL_CTX *ctx, const vector<unsigned int> &portnum,
               const SystemData &SD,
               Machine &machine, offline_control_data &OCD,
               int verbose);
#endif
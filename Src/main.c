/*
 * Copyright (C) 2021 University of Bologna
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 *
 * Authors: Alessio Burrello, UniBo (<alessio.burrello@unibo.it>)
 *          Nazareno Bruschi, UniBo (<nazareno.bruschi@unibo.it>)
 *          Davide Nadalini,  UniBo (<d.nadalini@unibo.it>)
 *          Francesco Conti,  UniBo (<f.conti@unibo.it>)
 */

#include "main.h"
int NETWORK_IDS[NB_LAYER] = {0};
int NETWORK_TYPES[NB_LAYER] = {CONV};

#define CHANNELS 1
#define SPATIAL_DIM 1

int NETWORK_DIMS[NB_LAYER][NB_DIM_ARGS] = {{SPATIAL_DIM, //x_in
                                            SPATIAL_DIM, //y_in
                                            CHANNELS, //c_in
                                            SPATIAL_DIM, //x_out
                                            SPATIAL_DIM, //y_out
                                            CHANNELS, //c_out
                                            1,  //x_stride
                                            1,  //y_stride
                                            0,  //top_pad
                                            0,  //bot_pad
                                            0,  //lef_pad
                                            0,  //rig_pad
                                            1,  //x_ker
                                            1}};//y_ker
PI_L2 network_layer_t network_layers[NB_LAYER];
PI_L1 network_layer_t l1_layer;
struct pi_device cluster;

/*
 * Cluster initialization
 */
void cluster_init()
{
#if defined(DEBUG)
  printf("-> Entering Cluster Initialization...\n");
#endif
  /*
   * Alloc L1 space for tiles: BE CAREFUL -> L1 is very limited (64kB)
   */
  l1_layer.id   = LAYER_ID(0);
  l1_layer.type = LAYER_TYPE(0);
  
  /*
   * Local dimensions determines the tile strategy
   */
  l1_layer.layer_dim.c_in     = network_layers[0].layer_dim.c_in;
  l1_layer.layer_dim.c_out    = network_layers[0].layer_dim.c_out;
  l1_layer.layer_dim.x_ker    = network_layers[0].layer_dim.x_ker;
  l1_layer.layer_dim.y_ker    = network_layers[0].layer_dim.y_ker;
  l1_layer.layer_dim.top_pad  = network_layers[0].layer_dim.top_pad;
  l1_layer.layer_dim.bot_pad  = network_layers[0].layer_dim.bot_pad;
  l1_layer.layer_dim.lef_pad  = network_layers[0].layer_dim.lef_pad;
  l1_layer.layer_dim.rig_pad  = network_layers[0].layer_dim.rig_pad;
  l1_layer.layer_dim.x_stride = network_layers[0].layer_dim.x_stride;
  l1_layer.layer_dim.y_stride = network_layers[0].layer_dim.y_stride;

#ifdef EXERCISE3
  /* EXERCISE3: ADD TILING FACTOR */
  int TILING_PARAMETER =  /* YOUR_CODE_HERE */;
  l1_layer.layer_dim.x_in  = network_layers[0].layer_dim.x_in  / TILING_PARAMETER;
  l1_layer.layer_dim.y_in  = network_layers[0].layer_dim.y_in  / TILING_PARAMETER;
  l1_layer.layer_dim.x_out = network_layers[0].layer_dim.x_out / TILING_PARAMETER;
  l1_layer.layer_dim.y_out = network_layers[0].layer_dim.y_out / TILING_PARAMETER;
#else
  l1_layer.layer_dim.x_in  = network_layers[0].layer_dim.x_in  / 1;
  l1_layer.layer_dim.y_in  = network_layers[0].layer_dim.y_in  / 1;
  l1_layer.layer_dim.x_out = network_layers[0].layer_dim.x_out / 1;
  l1_layer.layer_dim.y_out = network_layers[0].layer_dim.y_out / 1;
#endif


/* EXERCISE1.1: ALLOCATE MEMORY FOR CHANNELS AND SPATIAL DIMENSIONS */
/**
 * Suggestion: the input/kernel/output sizes are stored in these variables:
 * Input:
 *    l1_layer.layer_dim.x_in
 *    l1_layer.layer_dim.y_in
 *    l1_layer.layer_dim.c_in
 * Kernel:
 *    l1_layer.layer_dim.x_ker
 *    l1_layer.layer_dim.y_ker
 * Output:
 *    l1_layer.layer_dim.x_out
 *    l1_layer.layer_dim.y_out
 *    l1_layer.layer_dim.c_out
 * Then multiply by  sizeof(unsigned char) to get the size in Byte
 */

#ifndef EXERCISE2
  l1_layer.input_data  = pi_l1_malloc(&cluster, (unsigned int) (/* YOUR_CODE_HERE */)  * sizeof(/* YOUR_CODE_HERE */);
  l1_layer.param_data  = pi_l1_malloc(&cluster, (unsigned int) (/* YOUR_CODE_HERE */)  * sizeof(/* YOUR_CODE_HERE */);
  l1_layer.output_data = pi_l1_malloc(&cluster, (unsigned int) (/* YOUR_CODE_HERE */)  * sizeof(/* YOUR_CODE_HERE */);
#endif


  /*
   * PULP-NN Conv kernel exploits im2col to reorder the input data: Take it into account for L1 space
   */
  if(l1_layer.type == CONV)
  {
#ifndef EXERCISE2
  	l1_layer.buffer_0 = pi_l1_malloc(&cluster, (unsigned int)(2 * l1_layer.layer_dim.x_ker * l1_layer.layer_dim.y_ker * l1_layer.layer_dim.c_in * NB_CORES * sizeof(unsigned char)));
#else
  	l1_layer.buffer_0 = pi_l2_malloc((unsigned int)(2 * l1_layer.layer_dim.x_ker * l1_layer.layer_dim.y_ker * l1_layer.layer_dim.c_in * NB_CORES * sizeof(unsigned char)));
#endif
  }

#if defined(DEBUG)
  printf("-> Exiting Cluster Initialization...\n");
#endif
}

/*
 * FC application entry point
 */
int main()
{
  printf("\nEntering Main. Checking for Exercise...\n");
#if defined EXERCISE1
    printf("Executing Exercise 1\n");
#elif defined EXERCISE2
    printf("Executing Exercise 2\n");
#elif defined EXERCISE3
    printf("Executing Exercise 3\n");
#else
  printf("No Exercise selected. Exiting...\n");
  return;
#endif

  /*
   * Initialize the network and copy data in L3
   */
  int err = network_init();
  if(err)
  {
    return err;
  }

  /*
   * Copy data of the single layer in L2 (if it is possible)
   */
  err = layer_init();
  if(err)
  {
    return err;
  }

  /*
   * Now you should have all the parameters in L2 to compute the output of the first layer
   */
  /* 
   * DESCRIPTION:
   * PULP Cluster has 8 identical cores that can share and compute heavy computational workload in parallel.
   * PULP Cluster is a PMSIS device. Open it and send the task to initialize it.
   */

  struct pi_cluster_conf cluster_conf;
   pi_cluster_conf_init(&cluster_conf);
   pi_open_from_conf(&cluster, &cluster_conf);
   if(pi_cluster_open(&cluster))
   {
     return -CLUSTER_ERROR;
   }

  struct pi_cluster_task cluster_task;
  pi_cluster_send_task_to_cl(&cluster, pi_cluster_task(&cluster_task, cluster_init, NULL));
  pi_cluster_send_task_to_cl(&cluster, pi_cluster_task(&cluster_task, layer_run, NULL));
  
  err = layer_check();
  if(err)
  {
  	return err;
  }

  err = layer_free();
  if(err)
  {
    return err;
  }

  err = network_free();
  if(err)
  {
    return err;
  }

#if defined(DEBUG)
  printf("Exiting Main...\n");
#endif
  return 0;
}

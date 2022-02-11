#!/bin/bash
gcc --std=gnu99 -g -Wall  -o keygen keygen.c
gcc --std=gnu99 -g -Wall  -o enc_client enc_client.c
gcc --std=gnu99 -g -Wall  -o enc_server enc_server.c
gcc --std=gnu99 -g -Wall  -o dec_client dec_client.c
gcc --std=gnu99 -g -Wall  -o dec_server dec_server.c
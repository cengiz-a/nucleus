#!/bin/sh
gcc server.c put.c get.c del.c -o server
gcc telend.c -o client

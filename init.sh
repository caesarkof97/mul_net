#!/bin/bash
cur_dir=`pwd`
cd $cur_dir/vnic 
./init.sh 
sleep 5
cd $cur_dir/car 
./init.sh 

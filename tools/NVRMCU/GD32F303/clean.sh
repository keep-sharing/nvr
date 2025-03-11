#!/bin/bash
cur_dir=$(cd "$(dirname "$0")"; pwd)
rm -rf $cur_dir/Debug/Exe/*.out
rm -rf $cur_dir/Debug/List/*
rm -rf $cur_dir/Debug/Obj/*
rm -rf $cur_dir/Debug/Obj/.ninja*
echo "clean done!"


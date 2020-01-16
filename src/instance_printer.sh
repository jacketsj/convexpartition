#!/bin/bash
for fullfile in ../in/*; do
  filename=$(basename -- "$fullfile")
  echo ${filename%.*}
done 

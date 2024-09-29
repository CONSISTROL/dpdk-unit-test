#!/bin/bash

rm -rf tmp/*
for param in {a..o}; do
    sh sdv.sh "$param" &
done

#!/bin/bash -x

sudo apt update
sudo apt install python3-pip
pip3 install --upgrade pip3 setuptools wheel
pip3 install matplotlib
python3 ./fig1.py

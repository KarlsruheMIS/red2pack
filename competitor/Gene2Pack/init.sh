#!/bin/bash

echo "Installing python dependencies for gen2pack"
rm -rf venv
python3 -m venv venv
source venv/bin/activate

pip3 install deap igraph
echo "Finished to install python dependencies."
echo "To enter virtual enviroment run:"
echo "source $(pwd)/venv/bin/activate"
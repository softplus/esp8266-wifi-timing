#!/bin/bash

chmod +x *.sh

# setup virtualenv -- recommended
virtualenv .venv
source .venv/bin/activate

pip install -r requirements.txt

deactivate

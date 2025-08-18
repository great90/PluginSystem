#!/bin/env bash

cd build/

ls . | egrep "^[^_].*" | xargs rm -rf

cd ..


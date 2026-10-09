#!/bin/bash

rm -rf ./build-raspi
mkdir ./build-raspi

cp -r ./raspi/app ./raspi/.env ./raspi/docker-compose.yml ./build-raspi
rm -rf ./build-raspi/app/node_modules

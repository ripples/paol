#!/bin/bash

#Installs nginx on device
sudo apt-get install nginx


sudo apt-get install python-software-properties
sudo apt-add-repository ppa:chris-lea/node.js
sudo apt-get update

#Install Node.js
sudo apt-get install nodejs

#Install NPM
sudo apt-get install npm

#Installs mongoDB
sudo apt-key adv --keyserver hkp://keyserver.ubuntu.com:80 --recv 7F0CEB10
echo 'deb http://downloads-distro.mongodb.org/repo/ubuntu-upstart dist 10gen' | sudo tee /etc/apt/sources.list.d/mongodb.list
sudo apt-get update
sudo apt-get install mongodb-org

#Install required packages
sudo npm install -g yo grunt-cli bower
sudo npm install
bower install

#Install Ruby
sudo apt-get install ruby-full build-essential
sudo apt-get install rubygems
sudo gem install compass
sudo gem install sass

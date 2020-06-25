<!---
  Copyright @ 2019 Audi AG. All rights reserved.
  
      This Source Code Form is subject to the terms of the Mozilla
      Public License, v. 2.0. If a copy of the MPL was not distributed
      with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
  
  If it is not possible or desirable to put the notice in a particular file, then
  You may include the notice in a location (such as a LICENSE file in a
  relevant directory) where a recipient would be likely to look for such a notice.
  
  You may add additional accurate notices of copyright ownership.
  -->
# FEP SDK Controller Library Changelog {#fep_sdk_controller_change_log}
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/en/1.0.0) and this project adheres to [Semantic Versioning](https://semver.org/lang/en).


Release Notes - FEP Controller Library - Version 1.1.1

## [1.1.1] - Mar-2020

### Bug

### Change
	* [FEPSDK-2343] - Use of 2.6.1 System Library
    * [FEPSDK-1314] - Prepare FEP SDK code to be distributed as OSS
    * [FEPSDK-2022] - Cleanup CMake and conan as preparation for the open source distribution

Release Notes - FEP Controller Library - Version 1.1.0

## [1.1.0] - Jan-2020

### Bug
    * [FEPSDK-1983] - wrong logging statement in fep_controller.cpp:391 - missed system name
    * [FEPSDK-2059] - Syntax of Property Values to set and get is not clearly defined

### Change
    * [FEPSDK-1920] - Copy the configureTiming and the configureSystemProperties functionality from controller lib to system lib
    * [FEPSDK-1984] - Use fep3::System instead of std::unique_ptr<fep3::System> for connectSystem() as return value


Release Notes - FEP Controller Library - Version 1.0.0

## [1.0.0] - Oct-2019

### Change
    * [FEPSDK-1287] - System Library - add the possibility to configure a Timing Master - use AutomationInterface under the hood
    * [FEPSDK-1333] - Add "configure System Properties" functionality to controller lib  
    * [FEPSDK-1334] - Add "configure System Timing" functionality to controller lib  
    * [FEPSDK-1335] - Add "connectSystem" functionality to controller lib  

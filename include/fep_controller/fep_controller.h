/**

   @copyright
   @verbatim
   Copyright @ 2019 Audi AG. All rights reserved.
   
       This Source Code Form is subject to the terms of the Mozilla
       Public License, v. 2.0. If a copy of the MPL was not distributed
       with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
   
   If it is not possible or desirable to put the notice in a particular file, then
   You may include the notice in a location (such as a LICENSE file in a
   relevant directory) where a recipient would be likely to look for such a notice.
   
   You may add additional accurate notices of copyright ownership.
   @endverbatim
 */
#pragma once

#include <fep_system/fep_system.h>
#include <fep_controller/fep_controller_export.h>
#include <string>
#include <memory>

namespace fep
{   
    namespace controller
    {
        /**
         * Connects to a FEP System defined by a system sdk description
         *
         * @param [in] system_sdk_description_file The filepath to the system sdk description file
         *
         * @return Returns the connected system
         * @throws std::runtime_error if @p system_sdk_description_file can not be read
         *                            if the data model can not be created from @p system_sdk_description_file
         */
        fep::System FEP_CONTROLLER_EXPORT connectSystem(const std::string& system_sdk_description_file);

        /**
         * Sets the properties configured by @p system_properties_file for the @p system 
         *
         * @param [in] system The system for which the properties should be set
         * @param [in] system_properties_file The filepath to the system properties file
         *
         * @return Returns false if the file cannot be loaded or parsed or the system is not in the state FS_IDLE
         * @throws std::runtime_error if @p system_properties_file can not be read
         *                            if the data model can not be created from @p system_properties_file
         *                            if the system @p system is not is state FS_IDLE 
         */
        bool FEP_CONTROLLER_EXPORT configureSystemProperties(fep::System& system,
                                                             const std::string& system_properties_file);
    } // namespace controller
} // namespace fep

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
#include "fep_controller/fep_controller.h"
#include <fep_metamodel/fep_system.h>
#include <fep_system/base/properties/property_type.h> 
#include <a_util/xml.h>
#include <a_util/filesystem.h>

#include <algorithm>

using namespace fep::metamodel;

/*
class FEP_CONTROLLER_EXPORT fep_controller_dummyclass
{
    fep_controller_dummyclass() {};
};*/

namespace fep
{
namespace controller
{
namespace detail
{
    a_util::filesystem::Path normalizeToAnotherPath(const a_util::filesystem::Path& file_reference,
                                                    const a_util::filesystem::Path& other_file_path)
    {
        a_util::filesystem::Path file_path_normalized = file_reference;
        
        if (!file_path_normalized.isEmpty())
        {
            std::string file_path_normalized_as_string = file_path_normalized;
            a_util::strings::trim(file_path_normalized_as_string);
            file_path_normalized = file_path_normalized_as_string;
            if (file_path_normalized_as_string[0] == '$' && file_path_normalized_as_string[1] == '(')
            {
                //this might be a macro at the beginning !! do not change it 
                //this macro support is for further use!
                return file_path_normalized;
            }
            else
            {
                if (file_reference.isRelative())
                {
                    file_path_normalized = other_file_path;
                    file_path_normalized.append(file_reference);
                }
                file_path_normalized.makeCanonical();
                return file_path_normalized;
            }
        }
        else
        {
            return file_reference;
        }
    }
}

fep::System connectSystem(const std::string& system_description_file)
{   
    FepSystem system_sdk_file;
    a_util::xml::DOM dom;
    if (!dom.load(system_description_file))
    {
        throw std::runtime_error(a_util::strings::format("xml parse error for file '%s' : ",
            system_description_file.c_str(),
            dom.getLastError().c_str()));
    }

    // Parse data object model
    if (!system_sdk_file.internalReadConfig(dom))
    {
        std::string guess = "check mappings or timing node of the element_instance, if no file reference is set you need to remove the whole tag";
        throw std::runtime_error(a_util::strings::format("fep sdk system data model parse error for file '%s' : %s",
            system_description_file.c_str(),
            guess.c_str()));
    }

    //retrieve the Path for the system SDK 
    a_util::filesystem::Path system_sdk_file_path = system_description_file;
    system_sdk_file_path.makeCanonical();
    if (system_sdk_file_path.isRelative())
    {
        a_util::filesystem::Path working_path = a_util::filesystem::getWorkingDirectory();
        working_path.append(system_sdk_file_path);
        system_sdk_file_path = working_path;
    }
    system_sdk_file_path = system_sdk_file_path.getParent();

    //create the system
   fep::System system(system_sdk_file._name);

    for (FepParticipant& participant : system_sdk_file._participants)
    {
        system.add(participant._element_instance._id);
        auto part = system.getParticipant(participant._element_instance._id);
        part.setInitPriority(participant._init_priority);
        part.setStartPriority(participant._start_priority);
        //this will save the information for a possible configureSystem call
        if (participant._element_instance._timing)
        {
            a_util::filesystem::Path timing_file_reference = participant._element_instance._timing->_file_reference;
            //if a relative path is used within the file we make it relative to the system_file!!
            timing_file_reference = detail::normalizeToAnotherPath(timing_file_reference, system_sdk_file_path);
            part.setAdditionalInfo("timing_file_reference", timing_file_reference);
        }
        //this will save the information for a possible configureSystem call
        if (participant._element_instance._input_mapping)
        {
            a_util::filesystem::Path input_mapping = participant._element_instance._input_mapping->_file_reference;
            //if a relative path is used within the file we make it relative to the system_file!!
            input_mapping = detail::normalizeToAnotherPath(input_mapping, system_sdk_file_path);
            part.setAdditionalInfo("input_mapping", input_mapping);
        }
        //this will save the information for a possible configureSystem call
        if (participant._element_instance._output_mapping)
        {
            a_util::filesystem::Path output_mapping = participant._element_instance._output_mapping->_file_reference;
            //if a relative path is used within the file we make it relative to the system_file!!
            //I know there are problems when using 
            output_mapping = detail::normalizeToAnotherPath(output_mapping, system_sdk_file_path);
            part.setAdditionalInfo("output_mapping", output_mapping);
        }
    }

    return std::move(system);
}

std::string getValueFromProperty(const std::vector<fep::metamodel::Property>& properties,
    const std::string& key,
    const std::string& default_value)
{
    for (const auto& prop : properties)
    {
        if (prop._name == key)
        {
            return prop._value;
        }
    }
    return default_value;
}

bool configureSystemTimingFEP2(fep::System& system,
                               std::string& timing_type,
                               const std::vector<fep::metamodel::Property>& timing_props)
{
    if (timing_type == "Timing2NoMaster")
    {
        system.configureTiming2NoMaster();
    }
    else if (timing_type == "Timing2SystemTime")
    {
        auto master_element_id = getValueFromProperty(timing_props, "master_element_id", "");
        auto speed_factor = getValueFromProperty(timing_props, "master_time_factor", "1.0");
        if (master_element_id.empty())
        {
            throw std::runtime_error(a_util::strings::format("for timing type %s within system %s the 'master_element_id' is not set",
                timing_type.c_str(),
                system.getSystemName().c_str()));
        }
        else
        {
            system.configureTiming2SystemTime(master_element_id, speed_factor);
        }
    }
    else if (timing_type == "Timing2AFAP")
    {
        auto master_element_id = getValueFromProperty(timing_props, "master_element_id", "");
        if (master_element_id.empty())
        {
            throw std::runtime_error(a_util::strings::format("for timing type %s within system %s the 'master_element_id' is not set",
                timing_type.c_str(),
                system.getSystemName().c_str()));
        }
        else
        {
            system.configureTiming2AFAP(master_element_id);
        }
    }
    else
    {
        throw std::runtime_error(a_util::strings::format("unsupported timing type %s within system %s",
                                                        timing_type.c_str(),
                                                        system.getSystemName().c_str()));
    }
    return true;
}

bool configureSystemTimingFEP3(fep::System& system,
                                std::string& timing_type,
                                const std::vector<fep::metamodel::Property>& timing_props)
{
    //retrieve the infos master
    auto master_element_id = getValueFromProperty(timing_props, "master_element_id", "");
    auto master_clock_name = getValueFromProperty(timing_props, "master_clock_name", "local_system_realtime");
    auto master_time_stepsize = getValueFromProperty(timing_props, "master_time_stepsize", "100");
    auto master_time_factor = getValueFromProperty(timing_props, "master_time_factor", "1.0");
    //retrieve the infos slave
    auto slave_time_stepsize = getValueFromProperty(timing_props, "slave_time_stepsize", "100");

    if (timing_type == "Timing3NoMaster")
    {
        system.configureTiming3NoMaster();
    }
    else if (timing_type == "Timing3ClockSyncOnlyInterpolation")
    {
        system.configureTiming3ClockSyncOnlyInterpolation(master_element_id, slave_time_stepsize);
    }
    else if (timing_type == "Timing3ClockSyncOnlyDiscrete")
    {
        system.configureTiming3ClockSyncOnlyDiscrete(master_element_id, slave_time_stepsize);
    }
    else if (timing_type == "Timing3DiscreteSteps")
    {
        system.configureTiming3DiscreteSteps(master_element_id, master_time_stepsize, master_time_factor);
    }
    else if (timing_type == "Timing3AFAP")
    {
        system.configureTiming3AFAP(master_element_id, master_time_stepsize);
    }
    else
    {
        throw std::runtime_error(a_util::strings::format("unsupported timing type %s within system %s",
            timing_type.c_str(),
            system.getSystemName().c_str()));
    }
    return true;
}

bool configureSystemTiming(fep::System& system,
                           const std::vector<fep::metamodel::Property>& timing_props)
{
    
    //have a look into the XSD which type is supported
    auto timing_type = getValueFromProperty(timing_props, "timing_configuration_type", "PropertyBased");
    if (timing_type == "PropertyBased")
    {
        //we do nothing 
        //the timing is setup by the given properties
    }
    else if (timing_type.find("Timing2") == 0)
    {
        return configureSystemTimingFEP2(system, timing_type, timing_props);
    }
    else if (timing_type.find("Timing3") == 0)
    {
        return configureSystemTimingFEP3(system, timing_type, timing_props);
    }
    else
    {
        throw std::runtime_error(a_util::strings::format("unsupported timing type %s within system %s",
                                 timing_type.c_str(),
                                 system.getSystemName().c_str()));
    }
    return true;
}

bool configureSystemProperties(fep::System& system, const std::string& system_properties_file)
{
    PropertyFile property_file;
    a_util::xml::DOM dom;
    if (!dom.load(system_properties_file))
    {
        throw std::runtime_error(a_util::strings::format("xml parse error for file '%s' : ",
            system_properties_file.c_str(),
            dom.getLastError().c_str()));
    }

    // Parse data object model
    if (!property_file.internalReadConfig(dom))
    {
        throw std::runtime_error(a_util::strings::format("fep sdk system properties data model parse error for file '%s' : ",
            system_properties_file.c_str(),
            ""));
    }

    if (system.getSystemState() != fep::FS_IDLE)
    {
        throw std::runtime_error(a_util::strings::format("the system %s must be stopped! and set to idle state",
            system.getSystemName().c_str()));
    }

    std::vector<fep::ParticipantProxy> participants;
    participants = system.getParticipants();
    for (fep::ParticipantProxy& participant : participants)
    {
        auto config_rpc_client = participant.getRPCComponentProxy<fep::rpc::IRPCConfiguration>();
        fep::rpc::IRPCConfiguration& config_rpc_intf = config_rpc_client.getInterface();
        std::shared_ptr<fep::IProperties> participant_properties_node;
        try
        {
            participant_properties_node = config_rpc_intf.getProperties("/");
        }
        catch (const std::runtime_error&)
        {
            //accept not found
        }
        
        std::shared_ptr<fep::IProperties> system_properties_node;
        try
        {
            system_properties_node = config_rpc_intf.getProperties("/system");
        }
        catch (const std::runtime_error&)
        {
            //accept not found
        }
        if (system_properties_node)
        {
            // Set system properties
            for (Property& file_property : property_file._system_properties)
            {
                system_properties_node->setProperty(file_property._name, file_property._value, file_property._type);
            }
        }

        // Find the first element in the property file with the same name as the participant
        auto it = std::find_if(property_file._element_instances_properties.begin(),
            property_file._element_instances_properties.end(),
            [&participant](const PropertyFile::ElementInstance& element) {return element._id == participant.getName();}
        );

        if (participant_properties_node)
        {
            // Set element instance properties
            if (it != property_file._element_instances_properties.end())
            {
                for (Property& file_property : it->_properties)
                {
                    if (!participant_properties_node->setProperty(file_property._name, file_property._value, file_property._type))
                    {
                        throw std::runtime_error(a_util::strings::format("Error setting property '%s'.",
                            file_property._name.c_str()));
                    }
                }
            }
        }
    }

    return configureSystemTiming(system, property_file._system_timing_properties);
}
}
}

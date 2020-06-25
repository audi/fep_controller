/**
 * Implementation of the tester for the FEP Data Sample (locking)
 *
 * @file

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
 *
 *
 * @remarks
 *
 */

 /**
 * Test Case:   TestSystemLibrary
 * Test ID:     1.0
 * Test Title:  FEP System Library base test
 * Description: Test if controlling a test system is possible
 * Strategy:    Invoke Controller and issue commands
 * Passed If:   no errors occur
 * Ticket:      -
 * Requirement: -
 */

#include <gtest/gtest.h>
#include <fep_controller/fep_controller.h>
#include <fep_system/fep_system.h>
#include <a_util/system.h>
#include <a_util/xml.h>
#include <a_util/filesystem.h>

#include <fep_participant_sdk.h>
#include <fep3/participant/default_participant.h>

#include <string>
#include <mutex>
#include <memory>

inline void setModuleHeaderInformation(fep::IParticipant& mod,
    const char* header_description,
    const char* header_type_id)
{
    fep::setProperty(mod, FEP_PARTICIPANT_HEADER_VERSION, 1.0);
    fep::setProperty(mod, FEP_PARTICIPANT_HEADER_DESCRIPTION, header_description);
    auto fFepVersion =
        static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MAJOR) + static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MINOR) / 10;
    fep::setProperty(mod, FEP_PARTICIPANT_HEADER_FEP_VERSION, fFepVersion);
    fep::setProperty(mod, FEP_PARTICIPANT_HEADER_CONTEXT, "FEP SDK");
    fep::setProperty(mod, FEP_PARTICIPANT_HEADER_CONTEXT_VERSION, 1.0);
    fep::setProperty(mod, FEP_PARTICIPANT_HEADER_VENDOR, "Audi Electronics Venture GmbH");
    fep::setProperty(mod, FEP_PARTICIPANT_HEADER_TYPE_ID, header_type_id);
}

/**
 * @req_id ""
 */
TEST(TesterControllerLib, testConnectSystemFileNotExisting)
{
    try 
    {
        fep::controller::connectSystem("file_does_not_exists");
        FAIL() << "Expected std::runtime_error";
    }
    catch (std::runtime_error const & err)
    {
        std::string error_what = err.what();
        ASSERT_NE(error_what.find(std::string("file_does_not_exists"), 0), std::string::npos);
        ASSERT_NE(error_what.find(std::string("xml parse error"), 0), std::string::npos);
    }
    catch (...)
    {
        FAIL() << "Expected std::runtime_error";
    }
}

/**
 * @req_id ""
 */
TEST(TesterControllerLib, testConnectSystem)
{
    std::list<std::shared_ptr<fep::ParticipantFEP2>> lst_parts;
    for (size_t val = 0; val < 10; val++)
    {
        auto part = std::make_shared<fep::ParticipantFEP2>();
        setModuleHeaderInformation(*part, "bla", "bla");
        part->Create(a_util::strings::format("test_part_%d", val).c_str());
        lst_parts.push_back(part);
    }
    a_util::filesystem::Path test_file(TESTFILES_DIR);
    test_file.append("files/10_participants.fep_sdk_system");    

    std::unique_ptr<fep::System> system_to_test;
    ASSERT_NO_THROW(system_to_test = std::unique_ptr<fep::System>(
        new fep::System(
            std::move(fep::controller::connectSystem(test_file)))));

    ASSERT_EQ(system_to_test->getSystemName(), "FEP_SYSTEM");

    size_t counter = 0;
    for (auto part_proxy : system_to_test->getParticipants())
    {
        ASSERT_EQ(part_proxy.getName(), a_util::strings::format("test_part_%d", counter).c_str());
        ASSERT_EQ(part_proxy.getStartPriority(), counter);
        ASSERT_EQ(part_proxy.getInitPriority(), counter);
        auto stm = part_proxy.getRPCComponentProxyByIID<fep::rpc::IRPCStateMachine>();
        ASSERT_TRUE(static_cast<bool>(stm));
        ASSERT_EQ(stm->getState(), fep::rpc::IRPCStateMachine::State::FS_IDLE);
        counter++;
    }

    system_to_test->shutdown();
}

/**
 * @req_id ""
 */
TEST(TesterControllerLib, testConfigureSystem)
{
    std::list<std::shared_ptr<fep::ParticipantFEP2>> lst_parts;
    for (size_t val = 0; val < 2; val++)
    {
        auto part = std::make_shared<fep::ParticipantFEP2>();
        setModuleHeaderInformation(*part, "bla", "bla");
        part->Create(a_util::strings::format("test_part_%d", val).c_str());

        //keep in mind ... if changig this setup of test .. you need to change the property file too!
        //system properties must be set (config interface will not create properties!)
        fep::setProperty(*fep::getComponent<fep::IPropertyTree>(part->getComponents()), "system.system_parameter", 1);
        
        //part1
        fep::setProperty(*fep::getComponent<fep::IPropertyTree>(part->getComponents()), "pos1", 0.0);
        fep::setProperty(*fep::getComponent<fep::IPropertyTree>(part->getComponents()), "bool.value", false);
        fep::setProperty(*fep::getComponent<fep::IPropertyTree>(part->getComponents()), "double.pos2", 1.1);
        fep::setProperty(*fep::getComponent<fep::IPropertyTree>(part->getComponents()), "string.test", "empty");
        fep::setProperty(*fep::getComponent<fep::IPropertyTree>(part->getComponents()), "parameter1", 1);

        //part2
        fep::setProperty(*fep::getComponent<fep::IPropertyTree>(part->getComponents()), "pos.X", 11);
        lst_parts.push_back(part);
    }
    a_util::filesystem::Path test_file_system(TESTFILES_DIR);
    test_file_system.append("files/2_participants.fep_sdk_system");

    a_util::filesystem::Path test_file_properties(TESTFILES_DIR);
    test_file_properties.append("files/2_participants.fep_system_properties");

    std::unique_ptr<fep::System> system_to_test;
    ASSERT_NO_THROW(system_to_test = std::unique_ptr<fep::System>(
        new fep::System(
            std::move(fep::controller::connectSystem(test_file_system)))));

    bool success_configured = false;
    ASSERT_NO_THROW(success_configured = fep::controller::configureSystemProperties(*system_to_test, test_file_properties));
    ASSERT_TRUE(success_configured);

    //prepare for test
    auto part0 = system_to_test->getParticipant("test_part_0");
    auto props_part0 = part0.getRPCComponentProxyByIID<fep::rpc::IRPCConfiguration>()->getProperties("/");
    auto props_sys_part0 = part0.getRPCComponentProxyByIID<fep::rpc::IRPCConfiguration>()->getProperties("/system");

    //part 0 props are changed !!
    EXPECT_EQ(a_util::strings::toDouble(props_part0->getProperty("pos1")), a_util::strings::toDouble("1.234"));
    EXPECT_EQ(props_part0->getProperty("bool/value"), "true");
    EXPECT_EQ(a_util::strings::toDouble(props_part0->getProperty("double/pos2")), a_util::strings::toDouble("0.00"));
    EXPECT_EQ(props_part0->getProperty("string/test"), "this is a string");
    EXPECT_EQ(props_part0->getProperty("parameter1"), "3");

    //part1
    EXPECT_EQ(props_part0->getProperty("pos/X"), "11");

    //system props
    EXPECT_EQ(props_sys_part0->getProperty("system_parameter"), "42");

    //test part 1 (part 0 props)
    auto part1 = system_to_test->getParticipant("test_part_1");
    auto props_part1 = part1.getRPCComponentProxyByIID<fep::rpc::IRPCConfiguration>()->getProperties("/");
    auto props_sys_part1 = part1.getRPCComponentProxyByIID<fep::rpc::IRPCConfiguration>()->getProperties("/system");

    //part 0 props are NOT changed on part 1
    EXPECT_EQ(a_util::strings::toDouble(props_part1->getProperty("pos1")), a_util::strings::toDouble("0.0"));
    EXPECT_EQ(props_part1->getProperty("bool/value"), "false");
    EXPECT_EQ(a_util::strings::toDouble(props_part1->getProperty("double/pos2")), a_util::strings::toDouble("1.1"));
    EXPECT_EQ(props_part1->getProperty("string/test"), "empty");
    EXPECT_EQ(props_part1->getProperty("parameter1"), "1");

    //part1
    EXPECT_EQ(props_part1->getProperty("pos/X"), "100");

    //system props
    EXPECT_EQ(props_sys_part1->getProperty("system_parameter"), "42");
}

/** @brief Test whether properties having a preceeding '/' may be set.
*   @req_id FEPSDK-2059
*/
/**
 * @req_id ""
 */
TEST(TesterControllerLib, testConfigureSystemRootProperties)
{
    std::list<std::shared_ptr<fep::ParticipantFEP2>> lst_parts;
    auto part = std::make_shared<fep::ParticipantFEP2>();
    setModuleHeaderInformation(*part, "bla", "bla");
    part->Create(a_util::strings::format("test_part").c_str());

    //keep in mind ... if changig this setup of test .. you need to change the property file too!
    //system properties must be set (config interface will not create properties!)
    fep::setProperty(*fep::getComponent<fep::IPropertyTree>(part->getComponents()), "bool.value", false);
    fep::setProperty(*fep::getComponent<fep::IPropertyTree>(part->getComponents()), "double.pos2", 1.2);
    fep::setProperty(*fep::getComponent<fep::IPropertyTree>(part->getComponents()), "string.test.pfad", "test");
    fep::setProperty(*fep::getComponent<fep::IPropertyTree>(part->getComponents()), "parameter1", 1);

    a_util::filesystem::Path test_file_system(TESTFILES_DIR);
    test_file_system.append("files/1_participant.fep_sdk_system");

    a_util::filesystem::Path test_file_properties(TESTFILES_DIR);
    test_file_properties.append("files/root_properties.fep_system_properties");

    fep::System system_to_test = fep::controller::connectSystem(test_file_system);
         
    EXPECT_NO_THROW(fep::controller::configureSystemProperties(system_to_test, test_file_properties));
    
    auto participant = system_to_test.getParticipant("test_part");
    auto properties = participant.getRPCComponentProxyByIID<fep::rpc::IRPCConfiguration>()->getProperties("/");

    EXPECT_EQ(properties->getProperty("/bool/value"), "true");
    EXPECT_EQ(a_util::strings::toDouble(properties->getProperty("/double/pos2/")), a_util::strings::toDouble("0.0"));
    EXPECT_EQ(properties->getProperty("/string/test/pfad"), "this is a string");
    EXPECT_EQ(properties->getProperty("/parameter1"), "3");
}

/** @brief Test whether setting a system properties file which contains
*   invalid formatted properties is refused.
*   Properties have to use a '/' delimiter instead of '.'.
*   @req_id FEPSDK-2059
*/
/**
 * @req_id ""
 */
TEST(TesterControllerLib, testConfigureSystemInvalidPropertyFormat)
{
    std::list<std::shared_ptr<fep::ParticipantFEP2>> lst_parts;
    auto part = std::make_shared<fep::ParticipantFEP2>();
    setModuleHeaderInformation(*part, "bla", "bla");
    part->Create(a_util::strings::format("test_part").c_str());

    //keep in mind ... if changig this setup of test .. you need to change the property file too!
    //system properties must be set (config interface will not create properties!)
    fep::setProperty(*fep::getComponent<fep::IPropertyTree>(part->getComponents()), "double.test", 0.0);

    a_util::filesystem::Path test_file_system(TESTFILES_DIR);
    test_file_system.append("files/1_participant.fep_sdk_system");

    a_util::filesystem::Path test_file_properties(TESTFILES_DIR);
    test_file_properties.append("files/invalid_property_syntax.fep_system_properties");

    bool success_configured = false;
    auto system_to_test = fep::controller::connectSystem(test_file_system);
    EXPECT_THROW(success_configured = fep::controller::configureSystemProperties(system_to_test, test_file_properties), std::runtime_error);
    try
    {
        success_configured = fep::controller::configureSystemProperties(system_to_test, test_file_properties);
    }
    catch (const std::runtime_error& error)
    {
            EXPECT_NE(std::string(error.what()).find("Error setting property"), std::string::npos);
    }
}
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

class TesterControllerLibTiming : public ::testing::Test
{
protected:
    std::list<std::shared_ptr<fep::ParticipantFEP2>> lst_parts;
    std::unique_ptr<fep::System> system_to_test;
    a_util::filesystem::Path test_file_system{ TESTFILES_DIR };
    a_util::filesystem::Path test_file_properties{ TESTFILES_DIR };
    bool success_configured = false;
   
    void SetUp()
    {
        for (size_t val = 0; val < 2; val++)
        {
            auto part = std::make_shared<fep::ParticipantFEP2>();
            setModuleHeaderInformation(*part, "bla", "bla");
            part->Create(a_util::strings::format("test_part_%d", val).c_str());
            lst_parts.push_back(part);
        }

        test_file_system.append("files/2_participants.fep_sdk_system");
        ASSERT_NO_THROW(system_to_test = std::unique_ptr<fep::System>(
            new fep::System(
                std::move(fep::controller::connectSystem(test_file_system)))));
    }
};

/**
 * @req_id ""
 */
TEST_F(TesterControllerLibTiming, testConfigureTiming2SystemTime)
{
    TEST_REQ("");
  
    test_file_properties.append("files/2_participants_Timing2SystemTime.fep_system_properties");
    ASSERT_NO_THROW(success_configured = fep::controller::configureSystemProperties(*system_to_test, test_file_properties));
    ASSERT_TRUE(success_configured);

    auto first = lst_parts.begin();
    //expect to have the timing master set to part 1
    //this is test_part_0
    EXPECT_EQ(fep::getProperty(*fep::getComponent<fep::IPropertyTree>((*first)->getComponents()), FEP_CLOCKSERVICE_SLAVE_MASTER_PARTICIPANT, ""),
        "test_part_1");
    EXPECT_EQ(fep::getProperty(*fep::getComponent<fep::IPropertyTree>((*first)->getComponents()), FEP_CLOCKSERVICE_MAIN_CLOCK, ""),
        FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_MASTER_LOCKED_STEP_SIMTIME);
    EXPECT_EQ(fep::getProperty(*fep::getComponent<fep::IPropertyTree>((*first)->getComponents()), FEP_SCHEDULERSERVICE_SCHEDULER, ""),
        FEP_SCHEDULERSERVICE_SCHEDULER_VALUE_MASTER_LOCKED_STEP_SIMTIME);

    auto second = ++first;
    //this is test_part_1
    EXPECT_EQ(fep::getProperty(*fep::getComponent<fep::IPropertyTree>((*second)->getComponents()), FEP_CLOCKSERVICE_SLAVE_MASTER_PARTICIPANT, ""),
        "test_part_1");
    EXPECT_EQ(fep::getProperty(*fep::getComponent<fep::IPropertyTree>((*second)->getComponents()), FEP_TIMING_MASTER_TIME_FACTOR, 4.0),
        1.0);
    EXPECT_EQ(fep::getProperty(*fep::getComponent<fep::IPropertyTree>((*second)->getComponents()), FEP_CLOCKSERVICE_MAIN_CLOCK, ""),
        FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_MASTER_LOCKED_STEP_SIMTIME);
    EXPECT_EQ(fep::getProperty(*fep::getComponent<fep::IPropertyTree>((*second)->getComponents()), FEP_SCHEDULERSERVICE_SCHEDULER, ""),
        FEP_SCHEDULERSERVICE_SCHEDULER_VALUE_MASTER_LOCKED_STEP_SIMTIME);
}


/**
 * @req_id ""
 */
TEST_F(TesterControllerLibTiming, testConfigureTiming2NoMaster)
{
    test_file_properties.append("files/2_participants_Timing2NoMaster.fep_system_properties");
    ASSERT_NO_THROW(success_configured = fep::controller::configureSystemProperties(*system_to_test, test_file_properties));
    ASSERT_TRUE(success_configured);

    auto first = lst_parts.begin();
    EXPECT_EQ(fep::getProperty(*fep::getComponent<fep::IPropertyTree>((*first)->getComponents()), FEP_CLOCKSERVICE_SLAVE_MASTER_PARTICIPANT, ""),
        "");
    EXPECT_EQ(fep::getProperty(*fep::getComponent<fep::IPropertyTree>((*first)->getComponents()), FEP_CLOCKSERVICE_MAIN_CLOCK, ""),
        FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_MASTER_LOCKED_STEP_SIMTIME);
    EXPECT_EQ(fep::getProperty(*fep::getComponent<fep::IPropertyTree>((*first)->getComponents()), FEP_SCHEDULERSERVICE_SCHEDULER, ""),
        FEP_SCHEDULERSERVICE_SCHEDULER_VALUE_MASTER_LOCKED_STEP_SIMTIME);

    auto second = ++first;
    EXPECT_EQ(fep::getProperty(*fep::getComponent<fep::IPropertyTree>((*second)->getComponents()), FEP_CLOCKSERVICE_SLAVE_MASTER_PARTICIPANT, ""),
        "");
    EXPECT_EQ(fep::getProperty(*fep::getComponent<fep::IPropertyTree>((*second)->getComponents()), FEP_CLOCKSERVICE_MAIN_CLOCK, ""),
        FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_MASTER_LOCKED_STEP_SIMTIME);
    EXPECT_EQ(fep::getProperty(*fep::getComponent<fep::IPropertyTree>((*second)->getComponents()), FEP_SCHEDULERSERVICE_SCHEDULER, ""),
        FEP_SCHEDULERSERVICE_SCHEDULER_VALUE_MASTER_LOCKED_STEP_SIMTIME);
}

/**
 * @req_id ""
 */
TEST_F(TesterControllerLibTiming, testConfigureTiming3NoMaster)
{
    test_file_properties.append("files/2_participants_Timing3NoMaster.fep_system_properties");
    ASSERT_NO_THROW(success_configured = fep::controller::configureSystemProperties(*system_to_test, test_file_properties));
    ASSERT_TRUE(success_configured);

    auto first = lst_parts.begin();
    EXPECT_EQ(fep::getProperty(*fep::getComponent<fep::IPropertyTree>((*first)->getComponents()), FEP_CLOCKSERVICE_SLAVE_MASTER_PARTICIPANT, ""),
        "");
    EXPECT_EQ(fep::getProperty(*fep::getComponent<fep::IPropertyTree>((*first)->getComponents()), FEP_CLOCKSERVICE_MAIN_CLOCK, ""),
        FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_LOCAL_SYSTEM_REAL_TIME);
    EXPECT_EQ(fep::getProperty(*fep::getComponent<fep::IPropertyTree>((*first)->getComponents()), FEP_SCHEDULERSERVICE_SCHEDULER, ""),
        FEP_SCHEDULERSERVICE_SCHEDULER_VALUE_CLOCK_BASED);

    auto second = ++first;
    EXPECT_EQ(fep::getProperty(*fep::getComponent<fep::IPropertyTree>((*second)->getComponents()), FEP_CLOCKSERVICE_SLAVE_MASTER_PARTICIPANT, ""),
        "");
    EXPECT_EQ(fep::getProperty(*fep::getComponent<fep::IPropertyTree>((*second)->getComponents()), FEP_CLOCKSERVICE_MAIN_CLOCK, ""),
        FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_LOCAL_SYSTEM_REAL_TIME);
    EXPECT_EQ(fep::getProperty(*fep::getComponent<fep::IPropertyTree>((*second)->getComponents()), FEP_SCHEDULERSERVICE_SCHEDULER, ""),
        FEP_SCHEDULERSERVICE_SCHEDULER_VALUE_CLOCK_BASED);
}

/**
 * @req_id ""
 */
TEST_F(TesterControllerLibTiming, testConfigureTiming3AFAP)
{   
    test_file_properties.append("files/2_participants_Timing3AFAP.fep_system_properties");
    ASSERT_NO_THROW(success_configured = fep::controller::configureSystemProperties(*system_to_test, test_file_properties));
    ASSERT_TRUE(success_configured);

    auto first = lst_parts.begin();
    //expect to have the timing master set to part 1
    //this is test_part_0
    EXPECT_EQ(fep::getProperty(*fep::getComponent<fep::IPropertyTree>((*first)->getComponents()), FEP_CLOCKSERVICE_SLAVE_MASTER_PARTICIPANT, ""),
        "test_part_1");
    EXPECT_EQ(fep::getProperty(*fep::getComponent<fep::IPropertyTree>((*first)->getComponents()), FEP_CLOCKSERVICE_MAIN_CLOCK, ""),
        FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_SLAVE_MASTER_ONDEMAND_DISCRETE);
    EXPECT_EQ(fep::getProperty(*fep::getComponent<fep::IPropertyTree>((*first)->getComponents()), FEP_SCHEDULERSERVICE_SCHEDULER, ""),
        FEP_SCHEDULERSERVICE_SCHEDULER_VALUE_CLOCK_BASED);

    auto second = ++first;
    //this is test_part_1
    EXPECT_EQ(fep::getProperty(*fep::getComponent<fep::IPropertyTree>((*second)->getComponents()), FEP_CLOCKSERVICE_SLAVE_MASTER_PARTICIPANT, ""),
        "test_part_1");
    EXPECT_EQ(fep::getProperty(*fep::getComponent<fep::IPropertyTree>((*second)->getComponents()), FEP_CLOCKSERVICE_MAIN_CLOCK, ""),
        FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_LOCAL_SYSTEM_SIM_TIME);
    EXPECT_EQ(fep::getProperty(*fep::getComponent<fep::IPropertyTree>((*second)->getComponents()), FEP_SCHEDULERSERVICE_SCHEDULER, ""),
        FEP_SCHEDULERSERVICE_SCHEDULER_VALUE_CLOCK_BASED);
    EXPECT_EQ(fep::getProperty(*fep::getComponent<fep::IPropertyTree>((*second)->getComponents()), FEP_CLOCKSERVICE_MAIN_CLOCK_SIM_TIME_TIME_FACTOR, 10.0),
        0.0);
    EXPECT_EQ(fep::getProperty(*fep::getComponent<fep::IPropertyTree>((*second)->getComponents()), FEP_CLOCKSERVICE_MAIN_CLOCK_SIM_TIME_CYCLE_TIME, 10),
        50);
}

/**
 * @req_id ""
 */
TEST_F(TesterControllerLibTiming, testConfigureTiming3ClockSyncOnlyInterpolation)
{  
    test_file_properties.append("files/2_participants_Timing3ClockSyncOnlyInterpolation.fep_system_properties");
    ASSERT_NO_THROW(success_configured = fep::controller::configureSystemProperties(*system_to_test, test_file_properties));
    ASSERT_TRUE(success_configured);

    auto first = lst_parts.begin();
    //expect to have the timing master set to part 1
    //this is test_part_0
    EXPECT_EQ(fep::getProperty(*fep::getComponent<fep::IPropertyTree>((*first)->getComponents()), FEP_CLOCKSERVICE_SLAVE_MASTER_PARTICIPANT, ""),
        "test_part_1");
    EXPECT_EQ(fep::getProperty(*fep::getComponent<fep::IPropertyTree>((*first)->getComponents()), FEP_CLOCKSERVICE_MAIN_CLOCK, ""),
        FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_SLAVE_MASTER_ONDEMAND);
    EXPECT_EQ(fep::getProperty(*fep::getComponent<fep::IPropertyTree>((*first)->getComponents()), FEP_SCHEDULERSERVICE_SCHEDULER, ""),
        FEP_SCHEDULERSERVICE_SCHEDULER_VALUE_CLOCK_BASED);
    EXPECT_EQ(fep::getProperty(*fep::getComponent<fep::IPropertyTree>((*first)->getComponents()), FEP_CLOCKSERVICE_SLAVE_SYNC_CYCLE_TIME, 0),
        10);

    auto second = ++first;
    //this is test_part_1
    EXPECT_EQ(fep::getProperty(*fep::getComponent<fep::IPropertyTree>((*second)->getComponents()), FEP_CLOCKSERVICE_SLAVE_MASTER_PARTICIPANT, ""),
        "test_part_1");
    EXPECT_EQ(fep::getProperty(*fep::getComponent<fep::IPropertyTree>((*second)->getComponents()), FEP_CLOCKSERVICE_MAIN_CLOCK, ""),
        FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_LOCAL_SYSTEM_REAL_TIME);
    EXPECT_EQ(fep::getProperty(*fep::getComponent<fep::IPropertyTree>((*second)->getComponents()), FEP_SCHEDULERSERVICE_SCHEDULER, ""),
        FEP_SCHEDULERSERVICE_SCHEDULER_VALUE_CLOCK_BASED);
}

/**
 * @req_id ""
 */
TEST_F(TesterControllerLibTiming, testConfigureTiming3ClockSyncOnlyDiscrete)
{
    test_file_properties.append("files/2_participants_Timing3ClockSyncOnlyDiscrete.fep_system_properties");
    ASSERT_NO_THROW(success_configured = fep::controller::configureSystemProperties(*system_to_test, test_file_properties));
    ASSERT_TRUE(success_configured);

    auto first = lst_parts.begin();
    //expect to have the timing master set to part 1
    //this is test_part_0
    EXPECT_EQ(fep::getProperty(*fep::getComponent<fep::IPropertyTree>((*first)->getComponents()), FEP_CLOCKSERVICE_SLAVE_MASTER_PARTICIPANT, ""),
        "test_part_1");
    EXPECT_EQ(fep::getProperty(*fep::getComponent<fep::IPropertyTree>((*first)->getComponents()), FEP_CLOCKSERVICE_MAIN_CLOCK, ""),
        FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_SLAVE_MASTER_ONDEMAND_DISCRETE);
    EXPECT_EQ(fep::getProperty(*fep::getComponent<fep::IPropertyTree>((*first)->getComponents()), FEP_SCHEDULERSERVICE_SCHEDULER, ""),
        FEP_SCHEDULERSERVICE_SCHEDULER_VALUE_CLOCK_BASED);
    EXPECT_EQ(fep::getProperty(*fep::getComponent<fep::IPropertyTree>((*first)->getComponents()), FEP_CLOCKSERVICE_SLAVE_SYNC_CYCLE_TIME, 0),
        10);

    auto second = ++first;
    //this is test_part_1
    EXPECT_EQ(fep::getProperty(*fep::getComponent<fep::IPropertyTree>((*second)->getComponents()), FEP_CLOCKSERVICE_SLAVE_MASTER_PARTICIPANT, ""),
        "test_part_1");
    EXPECT_EQ(fep::getProperty(*fep::getComponent<fep::IPropertyTree>((*second)->getComponents()), FEP_CLOCKSERVICE_MAIN_CLOCK, ""),
        FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_LOCAL_SYSTEM_REAL_TIME);
    EXPECT_EQ(fep::getProperty(*fep::getComponent<fep::IPropertyTree>((*second)->getComponents()), FEP_SCHEDULERSERVICE_SCHEDULER, ""),
        FEP_SCHEDULERSERVICE_SCHEDULER_VALUE_CLOCK_BASED);
}

/**
 * @req_id ""
 */
TEST_F(TesterControllerLibTiming, testConfigureTiming3DiscreteSteps)
{
    test_file_properties.append("files/2_participants_Timing3DiscreteSteps.fep_system_properties");
    ASSERT_NO_THROW(success_configured = fep::controller::configureSystemProperties(*system_to_test, test_file_properties));
    ASSERT_TRUE(success_configured);

    auto first = lst_parts.begin();
    //expect to have the timing master set to part 1
    //this is test_part_0
    EXPECT_EQ(fep::getProperty(*fep::getComponent<fep::IPropertyTree>((*first)->getComponents()), FEP_CLOCKSERVICE_SLAVE_MASTER_PARTICIPANT, ""),
        "test_part_1");
    EXPECT_EQ(fep::getProperty(*fep::getComponent<fep::IPropertyTree>((*first)->getComponents()), FEP_CLOCKSERVICE_MAIN_CLOCK, ""),
        FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_SLAVE_MASTER_ONDEMAND_DISCRETE);
    EXPECT_EQ(fep::getProperty(*fep::getComponent<fep::IPropertyTree>((*first)->getComponents()), FEP_SCHEDULERSERVICE_SCHEDULER, ""),
        FEP_SCHEDULERSERVICE_SCHEDULER_VALUE_CLOCK_BASED);

    auto second = ++first;
    //this is test_part_1
    EXPECT_EQ(fep::getProperty(*fep::getComponent<fep::IPropertyTree>((*second)->getComponents()), FEP_CLOCKSERVICE_SLAVE_MASTER_PARTICIPANT, ""),
        "test_part_1");
    EXPECT_EQ(fep::getProperty(*fep::getComponent<fep::IPropertyTree>((*second)->getComponents()), FEP_CLOCKSERVICE_MAIN_CLOCK, ""),
        FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_LOCAL_SYSTEM_SIM_TIME);
    EXPECT_EQ(fep::getProperty(*fep::getComponent<fep::IPropertyTree>((*second)->getComponents()), FEP_SCHEDULERSERVICE_SCHEDULER, ""),
        FEP_SCHEDULERSERVICE_SCHEDULER_VALUE_CLOCK_BASED);
    EXPECT_EQ(fep::getProperty(*fep::getComponent<fep::IPropertyTree>((*second)->getComponents()), FEP_CLOCKSERVICE_MAIN_CLOCK_SIM_TIME_TIME_FACTOR, 10.0),
        0.5);
    EXPECT_EQ(fep::getProperty(*fep::getComponent<fep::IPropertyTree>((*second)->getComponents()), FEP_CLOCKSERVICE_MAIN_CLOCK_SIM_TIME_CYCLE_TIME, 10),
        50);
}
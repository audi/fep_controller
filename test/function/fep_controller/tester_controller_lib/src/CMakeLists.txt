#
# Copyright @ 2019 Audi AG. All rights reserved.
# 
#     This Source Code Form is subject to the terms of the Mozilla
#     Public License, v. 2.0. If a copy of the MPL was not distributed
#     with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
# 
# If it is not possible or desirable to put the notice in a particular file, then
# You may include the notice in a location (such as a LICENSE file in a
# relevant directory) where a recipient would be likely to look for such a notice.
# 
# You may add additional accurate notices of copyright ownership.
#
find_package(GTest REQUIRED ${gtest_search_mode})
add_executable(tester_controller_lib connect_system.cpp configure_system_timing.cpp)
add_test(NAME tester_controller_lib
         COMMAND tester_controller_lib
         WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../")
set_target_properties(tester_controller_lib PROPERTIES FOLDER test)

# fep_core link is needed because of helper library
target_link_libraries(tester_controller_lib PRIVATE fep_controller_lib fep_participant GTest::Main)
target_compile_definitions(tester_controller_lib PRIVATE TESTFILES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../")

fep_deploy_libraries(tester_controller_lib)
# copy library for integrated tests
if(WIN32)
    add_custom_command(TARGET tester_controller_lib POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:fep_controller_lib> $<TARGET_FILE_DIR:tester_controller_lib>
    )
    add_custom_command(TARGET tester_controller_lib POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:fep_system> $<TARGET_FILE_DIR:tester_controller_lib>
    )
endif()


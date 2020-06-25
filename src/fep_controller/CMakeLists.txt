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
#target name is fep_controller_lib 
#... because within fep_participant there is altready a controller target grr!!
set(FEP_CONTROLLER_LIBRARY fep_controller_lib)

#this how to use it while compiling this project
set(BUILD_INTERFACE_INCLUDES ${PROJECT_SOURCE_DIR}/include)
#this is how to use includes on the target
set(INSTALL_INTERFACE_INCLUDES include)

#BUILD_SHARED_LIBS will be used automatically to determine shared or static library (set by conan helper with the shared option)
add_library(${FEP_CONTROLLER_LIBRARY} SHARED
    fep_controller.cpp
    ${PROJECT_SOURCE_DIR}/include/fep_controller/fep_controller.h
)

target_include_directories(${FEP_CONTROLLER_LIBRARY} PUBLIC
    "$<BUILD_INTERFACE:${BUILD_INTERFACE_INCLUDES}>"
    "$<INSTALL_INTERFACE:${INSTALL_INTERFACE_INCLUDES}>"
)

target_compile_definitions(${FEP_CONTROLLER_LIBRARY} PRIVATE FEP_CONTROLLER_LIB_DO_EXPORT)
set_target_properties(${FEP_CONTROLLER_LIBRARY} PROPERTIES CXX_VISIBILITY_PRESET hidden)

set_target_properties(${FEP_CONTROLLER_LIBRARY} PROPERTIES
    OUTPUT_NAME ${FEP_CONTROLLER_LIBRARY}
    DEBUG_POSTFIX "d${FEP_CONTROLLER_VERSION_MAJOR}.${FEP_CONTROLLER_VERSION_MINOR}"
    RELEASE_POSTFIX "${FEP_CONTROLLER_VERSION_MAJOR}.${FEP_CONTROLLER_VERSION_MINOR}"
    RELWITHDEBINFO_POSTFIX "${FEP_CONTROLLER_VERSION_MAJOR}.${FEP_CONTROLLER_VERSION_MINOR}"
)

find_package(fep_system REQUIRED)
find_package(fep_metamodel REQUIRED)
find_package(a_util REQUIRED)

target_link_libraries(${FEP_CONTROLLER_LIBRARY}
    PUBLIC
        fep_system
    PRIVATE
        fep_metamodel
        a_util
)

install(
    TARGETS ${FEP_CONTROLLER_LIBRARY}
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION lib
)

install(
    FILES
        fep_controller.cpp
    DESTINATION
        src/fep_controller
)
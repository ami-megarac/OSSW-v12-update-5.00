/******************************************************************************
 *
 * INTEL CONFIDENTIAL
 *
 * Copyright 2020 Intel Corporation.
 *
 * This software and the related documents are Intel copyrighted materials,
 * and your use of them is governed by the express license under which they
 * were provided to you ("License"). Unless the License provides otherwise,
 * you may not use, modify, copy, publish, distribute, disclose or transmit
 * this software or the related documents without Intel's prior written
 * permission.
 *
 * This software and the related documents are provided as is, with no express
 * or implied warranties, other than those that are expressly stated in the
 * License.
 *
 ******************************************************************************/

#include "utils_dbusplus.hpp"

using PropertyValue =
    std::variant<bool, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t,
                 uint64_t, double, std::string>;

namespace crashdump
{
int getBMCVersionDBus(char* bmcVerStr, size_t bmcVerStrSize)
{
#ifndef SPX_BMC_ACD
    using ManagedObjectType = boost::container::flat_map<
        sdbusplus::message::object_path,
        boost::container::flat_map<
            std::string, boost::container::flat_map<
                             std::string, std::variant<std::string>>>>;

    if (bmcVerStr == nullptr)
    {
        return 1;
    }

    sdbusplus::bus::bus dbus = sdbusplus::bus::new_default_system();
    sdbusplus::message::message getObjects = dbus.new_method_call(
        "xyz.openbmc_project.Software.BMC.Updater",
        "/xyz/openbmc_project/software", "org.freedesktop.DBus.ObjectManager",
        "GetManagedObjects");
    ManagedObjectType bmcUpdaterIntfs;
    try
    {
        sdbusplus::message::message resp = dbus.call(getObjects);
        resp.read(bmcUpdaterIntfs);
    }
    catch (sdbusplus::exception_t& e)
    {
        return 1;
    }

    for (const std::pair<
             sdbusplus::message::object_path,
             boost::container::flat_map<
                 std::string, boost::container::flat_map<
                                  std::string, std::variant<std::string>>>>&
             pathPair : bmcUpdaterIntfs)
    {
        boost::container::flat_map<
            std::string,
            boost::container::flat_map<
                std::string, std::variant<std::string>>>::const_iterator
            softwareVerIt =
                pathPair.second.find("xyz.openbmc_project.Software.Version");
        if (softwareVerIt != pathPair.second.end())
        {
            boost::container::flat_map<std::string, std::variant<std::string>>::
                const_iterator purposeIt =
                    softwareVerIt->second.find("Purpose");
            if (purposeIt != softwareVerIt->second.end())
            {
                const std::string* bmcPurpose =
                    std::get_if<std::string>(&purposeIt->second);
                if (bmcPurpose->compare(
                        "xyz.openbmc_project.Software.Version.BMC"))
                {
                    boost::container::flat_map<
                        std::string, std::variant<std::string>>::const_iterator
                        versionIt = softwareVerIt->second.find("Version");
                    if (versionIt != softwareVerIt->second.end())
                    {
                        const std::string* bmcVersion =
                            std::get_if<std::string>(&versionIt->second);
                        if (bmcVersion != nullptr)
                        {
                            size_t copySize =
                                std::min(bmcVersion->size(), bmcVerStrSize - 1);
                            bmcVersion->copy(bmcVerStr, copySize);
                            return 0;
                        }
                    }
                }
            }
        }
    }
#endif
    return 1;
}

int getBIOSVersionDBus(char* biosVerStr, size_t biosVerStrSize)
{
#ifndef SPX_BMC_ACD
    PropertyValue biosSoftwareVersion{};
    if (biosVerStr == nullptr)
    {
        return 1;
    }
    sdbusplus::bus::bus dbus = sdbusplus::bus::new_default_system();
    auto method =
        dbus.new_method_call("xyz.openbmc_project.Settings",
                             "/xyz/openbmc_project/software/bios_active",
                             "org.freedesktop.DBus.Properties", "Get");
    method.append("xyz.openbmc_project.Software.Version", "Version");
    try
    {
        auto reply = dbus.call(method);
        reply.read(biosSoftwareVersion);
    }
    catch (sdbusplus::exception_t& e)
    {
        return 1;
    }
    std::string* biosVersion = &(std::get<std::string>(biosSoftwareVersion));
    if (biosVersion != nullptr)
    {
        size_t copySize = std::min(biosVersion->size(), biosVerStrSize - 1);
        biosVersion->copy(biosVerStr, copySize);
        return 0;
    }
#endif
    return 1;
}
#ifndef SPX_BMC_ACD
std::shared_ptr<sdbusplus::bus::match::match>
    startHostStateMonitor(std::shared_ptr<sdbusplus::asio::connection> conn)
{
    return std::make_shared<sdbusplus::bus::match::match>(
        *conn,
        "type='signal',interface='org.freedesktop.DBus.Properties',arg0='xyz."
        "openbmc_project.State.Host'",
        [](sdbusplus::message::message& msg) {
            std::string thresholdInterface;
            boost::container::flat_map<std::string, std::variant<std::string>>
                propertiesChanged;

            msg.read(thresholdInterface, propertiesChanged);

            if (propertiesChanged.empty())
            {
                return;
            }

            std::string event = propertiesChanged.begin()->first;

            auto variant =
                std::get_if<std::string>(&propertiesChanged.begin()->second);

            if (event.empty() || nullptr == variant)
            {
                return;
            }

            if (event == "CurrentHostState")
            {
                if (*variant == "xyz.openbmc_project.State.Host.HostState.Off")
                {
                    setResetDetected();
                    return;
                }
            }
        });
}
#endif
} // namespace crashdump
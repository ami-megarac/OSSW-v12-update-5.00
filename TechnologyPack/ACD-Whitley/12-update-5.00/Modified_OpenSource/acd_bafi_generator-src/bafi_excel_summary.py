##################################################################################
#
# INTEL CONFIDENTIAL
#
# Copyright 2021 Intel Corporation.
#
# This software and the related documents are Intel copyrighted materials, and
# your use of them is governed by the express license under which they were
# provided to you ("License"). Unless the License provides otherwise, you may
# not use, modify, copy, publish, distribute, disclose or transmit this
# software or the related documents without Intel's prior written permission.
#
# This software and the related documents are provided as is, with no express
# or implied warranties, other than those that are expressly stated in
# the License.
#
##################################################################################

from argparse import ArgumentParser
from platform import system
from os import path, walk
from subprocess import PIPE, run
from json import loads, decoder
from csv import DictWriter
from pathlib import Path
from sys import version_info


def get_crashdumps(folder):
    crashdump_folder = path.normpath(folder)
    crashdumps = []
    for dirpath, dirs, files in walk(crashdump_folder):
        for filename in files:
            if not filename.endswith(".json"):
                continue
            file_with_path = path.join(dirpath, filename)
            crashdumps.append(file_with_path)

    return crashdumps


def get_triage(file_name, binary_location, memory_map=None, silkscreen_map=None):
    binary_file = path.normpath(binary_location)
    if not memory_map and not silkscreen_map:
        output = run([binary_file, "--triage", file_name], stdout=PIPE)
    elif memory_map and not silkscreen_map:
        memory_map_file = path.normpath(memory_map)
        output = run([binary_file, "-m", memory_map_file, "--triage", file_name], stdout=PIPE)
    elif not memory_map and silkscreen_map:
        silkscreen_map_file = path.normpath(silkscreen_map)
        output = run([binary_file, "-s", silkscreen_map_file, "--triage", file_name], stdout=PIPE)
    else:
        memory_map_file = path.normpath(memory_map)
        silkscreen_map_file = path.normpath(silkscreen_map)
        output = run([binary_file, "-m", memory_map_file, "-s", silkscreen_map_file, "--triage", file_name], stdout=PIPE)
    try:
        json_data = loads(output.stdout)
    except decoder.JSONDecodeError:
        return None

    return json_data


def prepare_headers(max_sockets):
    sockets = [x for x in range(max_sockets)]
    fieldnames = ["File_Name", "First_Error_On_Socket"]
    for socket_number in sockets:
        fieldnames.append("Socket_{}_Error_Information".format(socket_number))
        fieldnames.append("Socket_{}_FRU".format(socket_number))
        fieldnames.append("Socket_{}_FRU_address".format(socket_number))
        fieldnames.append("Socket_{}_PPIN".format(socket_number))
        fieldnames.append("Socket_{}_Correctable_Memory_Errors".format(socket_number))
        fieldnames.append("Socket_{}_Uncorrectable_Memory_Errors".format(socket_number))
        fieldnames.append("Socket_{}_PCIE_AER_Correctable".format(socket_number))
        fieldnames.append("Socket_{}_PCIE_AER_Uncorrectable".format(socket_number))
        fieldnames.append("Socket_{}_Package_Thermal_Status".format(socket_number))

    return fieldnames, sockets


def save_to_csv(triage_data, output_file, max_sockets):
    fieldnames, sockets = prepare_headers(max_sockets)
    with open(path.normpath(output_file), "w", newline='') as fh:
        writer = DictWriter(fh, fieldnames=fieldnames)
        writer.writeheader()
        for dump_name, data in triage_data.items():
            row = {
                "File_Name": dump_name
            }
            if "error" in data.keys():
                row["First_Error_On_Socket"] = data["error"]
                writer.writerow(row)
                continue

            row["First_Error_On_Socket"] = data["First_Error_Occurred_On_Socket"]
            for socket_number in sockets:
                error_info = "Socket_{}_Error_Information".format(socket_number)
                socket_fru = "Socket_{}_FRU".format(socket_number)
                mem_err_corr = "Socket_{}_Correctable_Memory_Errors".format(socket_number)
                mem_err_uncor = "Socket_{}_Uncorrectable_Memory_Errors".format(socket_number)
                pcie_aer_corr = "Socket_{}_PCIE_AER_Correctable".format(socket_number)
                pcie_aer_uncor = "Socket_{}_PCIE_AER_Uncorrectable".format(socket_number)
                socket_fru_address = "Socket_{}_FRU_address".format(socket_number)
                package_thermal_status = "Socket_{}_Package_Thermal_Status".format(socket_number)
                ppin = "Socket_{}_PPIN".format(socket_number)
                if "socket{}".format(socket_number) in data.keys():
                    error_info_data = data["socket{}".format(socket_number)]["Error_Information"][0]
                    if data["socket{}".format(socket_number)]["FRU"] != {}:
                        socket_fru_data = data["socket{}".format(socket_number)]["FRU"]
                        for fru in data["socket{}".format(socket_number)]["FRU"]:
                            if isinstance(fru, dict) and "Address" in fru.keys():
                                socket_fru_address_data = fru["Address"]
                                del(fru["Address"])
                    else:
                        socket_fru_data = ""
                        socket_fru_address_data = ""
                    ppin_data = data["socket{}".format(socket_number)]["PPIN"]
                else:
                    error_info_data = ""
                    socket_fru_data = ""
                    ppin_data = ""
                    socket_fru_address_data = ""
                mem_err_corr_data = ""
                mem_err_uncor_data = ""
                if data["socket{}".format(socket_number)]["CorrectableMemoryErrors"]:
                    mem_err_corr_data = data["socket{}".format(socket_number)]["CorrectableMemoryErrors"]
                if data["socket{}".format(socket_number)]["UncorrectableMemoryErrors"]:
                    mem_err_uncor_data = data["socket{}".format(socket_number)]["UncorrectableMemoryErrors"]

                pcie_aer_corr_data = ""
                pcie_aer_uncor_data = ""
                if data["socket{}".format(socket_number)]["PcieAerCorrectable"]:
                    pcie_aer_corr_data = data["socket{}".format(socket_number)]["PcieAerCorrectable"]
                if data["socket{}".format(socket_number)]["PcieAerUncorrectable"]:
                    pcie_aer_uncor_data = data["socket{}".format(socket_number)]["PcieAerUncorrectable"]

                package_thermal_status_data = ""
                if data["socket{}".format(socket_number)]["PackageThermalStatus"]:
                    package_thermal_status_data = data["socket{}".format(socket_number)]["PackageThermalStatus"]

                row[error_info] = error_info_data
                row[socket_fru_address] = socket_fru_address_data
                row[socket_fru] = socket_fru_data
                row[ppin] = ppin_data
                row[mem_err_corr] = mem_err_corr_data
                row[mem_err_uncor] = mem_err_uncor_data
                row[pcie_aer_corr] = pcie_aer_corr_data
                row[pcie_aer_uncor] = pcie_aer_uncor_data
                row[package_thermal_status] = package_thermal_status_data
            writer.writerow(row)


def get_arguments():
    parser = ArgumentParser(
        epilog="Example usage: python3 bafi_excel_summary.py C:\\temp\crashdumps --max_sockets 2 --memory_map "
               "C:\\temp\default_memory_map.json -b C:\\temp\\bafi.exe -o C:\\temp\output_data.csv"
    )
    os_type = system()
    if os_type == "Windows":
        binary_file = path.join(str(Path(__file__).parent.absolute()), "bafi.exe")
    else:
        binary_file = path.join(str(Path(__file__).parent.absolute()), "bafi")
    parser.add_argument("-b", "--binary_location",
                        help="location of BAFI executable - by default bafi.exe (WIN) or bafi (LINUX) in current location",
                        default=binary_file)
    parser.add_argument("--max_sockets", type=int,
                        help="maximal number of sockets in crashdumps - default is 8", default=8)
    parser.add_argument("--memory_map", type=str,
                        help="memory map file that will be provided for BAFI", default=None)
    parser.add_argument("--silkscreen_map", type=str,
                        help="silkscreen map file that will be provided for BAFI", default=None)
    parser.add_argument("-o", "--output_file", type=str,
                        help="output csv file - default output.csv", default="output.csv")
    parser.add_argument("crashdumps_folder", type=str,
                        help="folder with crash dumps")

    return parser.parse_args()


def check_proper_python_version():
    if int(version_info.major) < 3:
        return False
    elif int(version_info.minor) < 6:
        return False

    return True


def main():
    if not check_proper_python_version():
        print("\nMinimum supported Python version is 3.6\n")
        exit(-1)
    filename_with_triage = {}
    args = get_arguments()
    crashdumps = get_crashdumps(args.crashdumps_folder)
    for crashdump in crashdumps:
        triage_data = get_triage(crashdump, args.binary_location, args.memory_map, args.silkscreen_map)
        if not triage_data:
            filename_with_triage[crashdump] = {"error": "Error parsing file"}
        else:
            filename_with_triage[crashdump] = triage_data
    save_to_csv(filename_with_triage, args.output_file, args.max_sockets)


if __name__ == '__main__':
    main()

/******************************************************************************
 *
 * INTEL CONFIDENTIAL
 *
 * Copyright 2020 Intel Corporation.
 *
 * This software and the related documents are Intel copyrighted materials, and
 * your use of them is governed by the express license under which they were
 * provided to you ("License"). Unless the License provides otherwise, you may
 * not use, modify, copy, publish, distribute, disclose or transmit this
 * software or the related documents without Intel's prior written permission.
 *
 * This software and the related documents are provided as is, with no express
 * or implied warranties, other than those that are expressly stated in
 * the License.
 *
 ******************************************************************************/

#include <iostream>
#include <string>
#include <regex>

#include <utils.hpp>

void findSocketsFromSummary(std::vector<std::string> &sockets, json reportJson)
{
  for (const auto& metaDataItem : reportJson["summary"].items())
  {
      if (startsWith(metaDataItem.key(), "socket"))
      {
          sockets.push_back(metaDataItem.key());
      }
  }
}

std::string findFirstErrorSocket(json reportJson, std::string &firstError)
{
  std::vector<std::string> sockets;
  bool ierr = false;
  bool mcerr = false;
  std::string firstSocket;

  findSocketsFromSummary(sockets, reportJson);

  for (const auto socket : sockets)
  {
    for (const auto [pciKey, pciVal] : reportJson["summary"][socket].items())
    {
      if (pciVal.contains("TSC"))
      {
        for (const auto [metaKey, metaVal] : pciVal.items())
        {
          for (const auto [tscKey, tscVal] : metaVal.items())
          {
            if (std::string(tscVal).find("(Occurred first between all TSCs)") !=
            std::string::npos)
            {
              if (std::string(tscVal).find("pcu_first_ierr_tsc_cfg") !=
              std::string::npos)
              {
                firstError = "FirstIERR";
                ierr = true;
              }
              else
              {
                firstError = "FirstMCERR";
                mcerr = true;
              }
              firstSocket = socket;
            }
          }
        }
      }
    }
  }
  if (ierr || mcerr)
    return firstSocket;
  else
    return "";
}

std::string findBankNumber(std::stringstream &ss)
{
  std::string bankNumber;
  int found;
  std::stringstream ss2;
  std::size_t left = ss.str().find("bank");
  if (left > ss.str().size())
    return "";
  ss2 << ss.str().substr(left, 7);
  while (!ss2.eof())
  {
    ss2 >> bankNumber;
    if (std::stringstream(bankNumber) >> found)
    {
      break;
    }
    bankNumber = "";
  }
  return bankNumber;
}

void getErrorStrings(std::string entry, std::string &ierr, std::string &mcerr)
{
  std::size_t left = std::string(entry).find("FirstIERR");

  std::size_t right = std::string(entry)
  .substr(left, std::string(entry).size()).find(", FirstMCERR");
  ierr = std::string(entry)
  .substr(left, right).substr(12);

  left = std::string(entry).find("FirstMCERR");
  mcerr = std::string(entry)
  .substr(left + 13, std::string(entry).size());
}

std::string determineFirstErrorType(std::string entry, std::string &ierr, std::string &mcerr)
{
  getErrorStrings(entry, ierr, mcerr);

  if (ierr != "0x0" && mcerr == "0x0")
    return "FirstIERR";
  else
    return "FirstMCERR";
}

void printTriage(json reportJson, const json& input)
{
  json triageReport;
  std::string firstError = "";
  std::stringstream ss;
  std::stringstream ss2;
  std::regex core_regex("([0-9])-([0-9])");
  std::smatch matches;
  bool imc_present = false;
  bool aer_section = false;
  bool fru_section = false;
  std::string imc_string;
  std::string bankNumber;
  std::string ierr;
  std::string mcerr;
  std::vector<std::string> sockets;
  std::string firstErrorSocket = findFirstErrorSocket(reportJson, firstError);

  if (firstErrorSocket != "")
  {
    triageReport["First_Error_Occurred_On_Socket"] = firstErrorSocket.substr(6);
    sockets.push_back(firstErrorSocket);
  }
  else
  {
    firstError = "FirstMCERR";
    findSocketsFromSummary(sockets, reportJson);
    if (sockets.size() > 1)
      triageReport["First_Error_Occurred_On_Socket"] = "N/A";
    else
      triageReport["First_Error_Occurred_On_Socket"] = "0";
  }

  for (const auto socket : sockets)
  {
    ss.str("");
    ss << "S" << socket.substr(6) << ".";
    for (const auto [pciKey, pciVal] : reportJson["summary"][socket].items())
    {
      if (pciVal.contains("Errors_Per_Socket"))
      {
        for (const auto [metaKey, metaVal] : pciVal.items())
        {
          for (const auto [tscKey, tscVal] : metaVal.items())
          {
            if (firstErrorSocket == "")
            {
              firstError = determineFirstErrorType(std::string(tscVal), ierr, mcerr);
              if (sockets.size() == 1 && ierr == "0x0" && mcerr == "0x0")
                triageReport["First_Error_Occurred_On_Socket"] = "N/A";
            }
            else
            {
              getErrorStrings(std::string(tscVal), ierr, mcerr);
            }
            std::size_t left = std::string(tscVal).find(firstError);
            if (left > std::string(tscVal).size())
              continue;
            if (firstError == "FirstIERR")
            {
              std::size_t right = std::string(tscVal)
              .substr(left, std::string(tscVal).size()).find(", FirstMCERR");
              if (ierr == "0x0")
              {
                ss << "N/A";
                break;
              }
              ss << "IERR.";
              if (std::string(tscVal).substr(left, right).find("IMC")
                != std::string::npos)
              {
                ss2 << std::string(tscVal).substr(left, right);
                imc_present = true;
                bankNumber = findBankNumber(ss2);
                break;
              }
              ss << ierr;
              bankNumber = findBankNumber(ss);
              break;
            }
            else
            {
              if (mcerr == "0x0")
              {
                ss << "N/A";
                break;
              }
              ss << "MCERR.";
              if (std::string(tscVal).substr(left, std::string(tscVal).size()).find("IMC")
                != std::string::npos)
              {
                ss2 << std::string(tscVal).substr(left, std::string(tscVal).size());
                imc_present = true;
                bankNumber = findBankNumber(ss2);
                break;
              }
              ss << mcerr;
              bankNumber = findBankNumber(ss);
              break;
            }
          }
        }
      }
    }

    for (const auto [pciKey, pciVal] : reportJson["summary"][socket].items())
    {
      if (pciVal.contains("MCE"))
      {
        for (const auto [metaKey, metaVal] : pciVal.items())
        {
          for (const auto [tscKey, tscVal] : metaVal.items())
          {
            if (bankNumber == "9" || bankNumber == "10" || bankNumber == "11")
            {
              if (std::string(tscVal).find("bank " + bankNumber)
                != std::string::npos)
              {
                std::size_t left = std::string(tscVal).find("RAW ");
                if (left > std::string(tscVal).size())
                  continue;
                std::size_t right = std::string(tscVal).find(std::string(tscVal).size());
                bankNumber = std::string(tscVal).substr(left + 4, left-right);
              }
            }
            std::string core_string = ss.str();
            if (std::regex_search(core_string, matches, core_regex))
            {
              for (int i = std::stoi(matches[1].str()); i <= std::stoi(matches[2].str()); i++)
              {
                if (std::string(tscVal).find("bank " + std::to_string(i)) !=
                  std::string::npos)
                {
                  bankNumber = std::to_string(i);
                  break;
                }
              }
            }
            if (imc_present)
            {
              if (std::string(tscVal).find("bank " + bankNumber)
                != std::string::npos)
              {
                std::size_t left = std::string(tscVal).find("bank");
                if (left > std::string(tscVal).size())
                  continue;
                std::size_t right = std::string(tscVal).find(")");
                imc_string = std::string(tscVal).substr(left, right + 1);
                ss << imc_string;
                bankNumber = findBankNumber(ss);
                break;
              }
              else
              {
                std::stringstream ss3;
                ss3 << ss2.str().erase(ss2.str().find("bank"), 3);
                ss2.str("");
                ss2 << ss3.str();
                bankNumber = findBankNumber(ss2);
              }
            }
          }
        }
      }
    }
    std::string mscod;
    std::string mcacod;
    std::string msec_fw = "";
    if (bankNumber.find("CHA") == std::string::npos)
      bankNumber = bankNumber + " ";

    for (const auto [pciKey, pciVal] : reportJson["MCA"][socket].items())
    {
      for (const auto [mcaKey, mcaVal] : pciVal.items())
      {
        if (mcaKey == "Bank")
        {
          if (std::string(mcaVal).rfind(bankNumber, 0) != 0 || bankNumber == "")
          {
            break;
          }
          if (bankNumber == "4")
          {
            mscod = "";
            mcacod = pciVal["Status_decoded"]["MCACOD"];
            msec_fw = pciVal["Status_decoded"]["MSEC_FW"];
            break;
          }
        }

        if (mcaKey == "Status")
        {
          mscod = std::string(pciVal["Status"]).substr(10, 4);
          mcacod = std::string(pciVal["Status"]).substr(14, 4);
          if (mcacod == "")
            break;
          while (mscod.front() == '0')
          {
            mscod.erase(0, 1);
            if (mscod == "")
              break;
          }
          while (mcacod.front() == '0')
          {
            mcacod.erase(0, 1);
            if (mcacod == "")
              break;
          }
          if (mscod == "")
          {
            mscod = "00";
          }
          if (mcacod == "")
          {
            mcacod = "00";
          }
          mscod = "0x" + mscod;
          mcacod = "0x" + mcacod;
          break;
        }
      }
    }

    if (mcacod != "")
    {
      if (msec_fw != "")
        ss << " MCACOD: " << mcacod << " MSEC_FW: " << msec_fw;
      else
        ss << " MCACOD: " << mcacod << " MSCOD: " << mscod;
    }

    json error_info = ss.str();
    triageReport[socket]["Error_Information"].push_back(error_info);
    std::stringstream ss_fru;
    std::stringstream ss_aer;

    if (imc_present)
    {
      std::size_t left = std::string(imc_string).find("bank");
      if (left <= std::string(imc_string).size())
      {
        std::size_t right = std::string(imc_string).find(")");
        ss_fru << std::string(imc_string).substr(left + 8, right + 1);
        json fru_info = ss_fru.str();
        triageReport[socket]["FRU"].push_back(fru_info);
        fru_section = true;
      }
    }

    for (const auto [pciKey, pciVal] : reportJson["summary"][socket].items())
    {
      if (pciVal.contains("PCIe_AER_Uncorrectable_errors"))
      {
        triageReport[socket]["PCIe_AER"]["Uncorrectable"]
        = pciVal["PCIe_AER_Uncorrectable_errors"];
        aer_section = true;
      }
      if (pciVal.contains("PCIe_AER_Correctable_errors"))
      {
        triageReport[socket]["PCIe_AER"]["Correctable"]
        = pciVal["PCIe_AER_Correctable_errors"];
        aer_section = true;
      }
      if (pciVal.contains("First_MCERR"))
      {
        auto fru_obj = json::object();
        fru_obj["Address"] = pciVal["First_MCERR"]["Address"];
        std::string bdf_string = pciVal["First_MCERR"]["BDF"];
        if (bdf_string == "Please refer to system address map")
        {
          fru_obj["Bus"] = bdf_string;
          fru_obj["Device"] = bdf_string;
          fru_obj["Function"] = bdf_string;
        }
        else
        {
          std::size_t left = bdf_string.find("Bus");
          std::size_t right = bdf_string.find(",");
          fru_obj["Bus"] = bdf_string.substr(left, right).substr(5);
          left = bdf_string.find("Device");
          right = bdf_string.substr(right + 2).find(",");
          fru_obj["Device"] = bdf_string.substr(left, right).substr(8);
          left = bdf_string.find("Function");
          fru_obj["Function"] = bdf_string.substr(left, bdf_string.size()).substr(10);
        }
        triageReport[socket]["FRU"].push_back(fru_obj);
        fru_section = true;
      }
      if (pciVal.contains("IO_Errors"))
      {
        auto fru_obj = json::object();
        std::string bdf_string = pciVal["IO_Errors"][0];
        std::size_t left = bdf_string.find("Bus");
        std::size_t right = bdf_string.find("] ");
        fru_obj["Bus"] = bdf_string.substr(left, right).substr(4);
        left = bdf_string.find("Device");
        right = bdf_string.substr(right + 2).find("]");
        fru_obj["Device"] = bdf_string.substr(left, right).substr(7);
        left = bdf_string.find("Function");
        fru_obj["Function"] = bdf_string
        .substr(left, bdf_string.size() - left - 1).substr(9);
        triageReport[socket]["FRU"].push_back(fru_obj);
        fru_section = true;
      }
    }
    if (!aer_section)
    {
      triageReport[socket]["PCIe_AER"] = json::object();
    }
    if (!fru_section)
    {
      triageReport[socket]["FRU"] = json::object();
    }
    aer_section = false;
    fru_section = false;

    if (input.find("METADATA") != input.cend())
    {
      if (!checkInproperValue(input["METADATA"]
      ["cpu" + socket.substr(6)]["ppin"]) && input["METADATA"]
      ["cpu" + socket.substr(6)]["ppin"] != "dummy")
      {
        triageReport[socket]["PPIN"] = input["METADATA"]
        ["cpu" + socket.substr(6)]["ppin"];
      }
      else
      {
        triageReport[socket]["PPIN"] = "N/A";
      }
    }
    else
    {
      triageReport[socket]["PPIN"] = json::object();
    }
  }
  std::cout << triageReport.dump(4) << std::endl;
}
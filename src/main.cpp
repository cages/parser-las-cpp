/*
   Las-Util - parse las formatted files and display the fields

   This file is part of the Las-Util-Cpp project

   Copyright (C) 2019 DC Slagel

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

   License-Identifier: Apache-2.0
*/


// ParseLas includes
#include "las.h"


int main(int argc, char *argv[])
{
    if (arg_parse(argc, argv) != 0) {
        exit(EXIT_FAILURE);
    }

    char *file_to_parse = get_filename_arg();


    std::ifstream DataSrc(file_to_parse);
    if (DataSrc.fail()) {
        std::cout << "ERROR: Cannot open ["
          << file_to_parse
          << "]   "
          << strerror(errno)
          <<  std::endl;
        exit(errno);
    }

    std::string line;

    /* ----------------------------------------------------
     * Get Version Section
     * ----------------------------------------------------*/

    LasVersion LVer;

    std::getline(DataSrc, line);
    trim(line);
    LVer.setHeader(line);

    std::getline(DataSrc, line);
    trim(line);
    LVer.setVersion(line);

    std::getline(DataSrc, line);
    trim(line);
    LVer.setLineWrap(line);

    std::getline(DataSrc, line);
    trim(line);

    // if the first character is '~' then the Version section is completed and
    // we are starting a new section.
    if (line[0] == '~') {
      if (is_section_printable('v')) {
        LVer.printInfo();
      }
    }
    else {
      LVer.setDelimiter(line);
    }

    /* ----------------------------------------------------
     * Get Well Section
     * ----------------------------------------------------*/
    LasWell LWell;

    LWell.setHeader(line);

    while (std::getline(DataSrc, line))
    {
        if (line[0] == '#') {
            continue;
        }

        // If we have a section header and the well section header
        // is already set (greater than zero) then we are at the next
        // section so break out of the well section processing
        // ----------------------------------------------------------
        if (line[0] == '~') {
            if (LWell.getHeader().size() > 0) {
                break;
            }
        }

        LWell.parseLine(line);
    }

    if (is_section_printable('w')) {
      LWell.printInfo();
    }

    /* ----------------------------------------------------
     * Get Curve Section
     * ----------------------------------------------------*/
    LasHeader CurveHeader;

    trim(line);
    if ( line[0] == '~' &&  (line[1] == 'C' || line[1] == 'c') ) {
        CurveHeader.parseLine(line);

        while (std::getline(DataSrc, line))
        {
            if (line[0] == '#') {
                continue;
            }

            if (line[0] == '~') {
                break;
            }

            CurveHeader.parseLine(line);
        }
        if (is_section_printable('c')) {
            CurveHeader.printInfo();
        }
    }

    /* ----------------------------------------------------
     * Get Parameter : Log_Parameter Section
     * ----------------------------------------------------*/
    // LasLogParam LLogParam;
    LasHeader ParamHeader;

    trim(line);
    std::string paramsectionindicator ("~P");

    if ( line.rfind(paramsectionindicator, 0) == 0 ) {
        ParamHeader.parseLine(line);

        while (std::getline(DataSrc, line))
        {
            if (line[0] == '#') {
                continue;
            }

            if (line[0] == '~') {
                break;
            }

            ParamHeader.parseLine(line);
        }

      if (is_section_printable('p')) {
          ParamHeader.printInfo();
      }
    }

    /* ----------------------------------------------------
     * Get Log_Definition Section
     * ----------------------------------------------------*/
    /*
    LasLogParam LLogDef;

    trim(line);
    if ( line == "~Log_Definition" ) {
        LLogDef.parseLine(line);

        while (std::getline(DataSrc, line))
        {
            if (line[0] == '#') {
                continue;
            }

            if (line[0] == '~') {
                break;
            }

            LLogDef.parseLine(line);
        }

        if (is_section_printable('d')) {
          LLogDef.printInfo();
        }
    }
    */

    /* ----------------------------------------------------
     * Get Other Section
     * ----------------------------------------------------*/
    LasOther OtherHeader;

    trim(line);
    std::string othersectionindicator ("~O");

    if ( line.rfind(othersectionindicator, 0) == 0 ) {
        OtherHeader.parseLine(line);

        while (std::getline(DataSrc, line))
        {
            if (line[0] == '#') {
                continue;
            }

            if (line[0] == '~') {
                break;
            }

            OtherHeader.parseLine(line);
        }

      if (is_section_printable('o')) {
          OtherHeader.printInfo();
      }
    }

    /* ----------------------------------------------------
     * Get Drilling_Definition Section
     * ----------------------------------------------------*/
    /*
    LasLogParam LDrillingDef;

    trim(line);
    if ( line == "~Drilling_Definition" ) {
        LDrillingDef.parseLine(line);

        while (std::getline(DataSrc, line))
        {
            if (line[0] == '#') {
                continue;
            }

            if (line[0] == '~') {
                break;
            }

            LDrillingDef.parseLine(line);
        }

        if (is_section_printable('e')) {
          LDrillingDef.printInfo();
        }
    }
    */

    /* ----------------------------------------------------
     * Get Drilling_Data Section
     *
     * ~Drilling_Data | Drilling_Definition
     * 322.02,1.02,0.0,24.0,3,59,111,1199,179, 879,8.73,39
     * 323.05,2.05,0.1,37.5,2,69,118,1182,175, 861,8.73,202
     *
     * ----------------------------------------------------*/
    /*
    trim(line);
    if ( line == "~Drilling_Data | Drilling_Definition" ) {
        if (is_section_printable('a')) {
          std::cout
              << "\n"
              << "# -------------------------------------------------"
              << "---------------------------------------------\n"
              << "#  " << "~Drilling_Data" << "\n"
              << "# -------------------------------------------------"
              << "---------------------------------------------\n";

          LDrillingDef.printDataHeader();
        }

        std::string field;
        while (std::getline(DataSrc, line))
        {
            trim(line);
            if (line[0] == '#' || line.size() == 0) {
                continue;
            }

            if (line[0] == '~') {
                break;
            }

            std::stringstream ss(line);
            std::getline(ss, field, ',');
            trim(field);

            if (is_section_printable('a')) {
              std::cout 
                  // << std::left
                  << std::setw(6)
                  << field
                  << " |";

              while (std::getline(ss, field, ',')) {
                  trim(field);
                  std::cout 
                      << std::right
                      << std::setw(6)
                      << field
                      << " |";
              }
              std::cout << "\n";
            }
        }
    }
    */


    DataSrc.close();

    return 0;
}

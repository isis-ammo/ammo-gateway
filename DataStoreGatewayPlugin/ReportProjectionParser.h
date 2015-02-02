/* Copyright (c) 2010-2015 Vanderbilt University
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef REPORT_PROJECTION_PARSER_H
#define REPORT_PROJECTION_PARSER_H

#include <string>

#include "StringParser.h"

class ReportProjectionParser : public StringParser
{
public:
  ReportProjectionParser (void);

  void parse (const std::string &params);

  std::string contentGuid_;
  std::string reportTime_min_;
  std::string reportTime_max_;
  std::string reportingUnit_;
  std::string size_min_;
  std::string size_max_;
  std::string activity_;
  std::string locationUtm_;
  std::string enemyUnit_;
  std::string observationTime_min_;
  std::string observationTime_max_;
  std::string equipment_;
  std::string assessment_;
  std::string narrative_;
  std::string authentication_;
};

#endif // REPORT_PROJECTION_PARSER_H



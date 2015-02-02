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

#ifndef EVENT_PROJECTION_PARSER_H
#define EVENT_PROJECTION_PARSER_H

#include <string>

#include "StringParser.h"

class EventProjectionParser : public StringParser
{
public:
  EventProjectionParser (void);

  void parse (const std::string &params);

  std::string uuid_;
  std::string mediaCount_min_;
  std::string mediaCount_max_;
  std::string displayName_;
  std::string categoryId_;
  std::string title_;
  std::string description_;
  std::string longitude_min_;
  std::string longitude_max_;
  std::string latitude_min_;
  std::string latitude_max_;
  std::string createdDate_min_;
  std::string createdDate_max_;
  std::string modifiedDate_min_;
  std::string modifiedDate_max_;
  std::string cid_;
  std::string category_;
  std::string unit_;
  std::string size_min_;
  std::string size_max_;
  std::string dest_group_type_;
  std::string dest_group_name_;
  std::string status_min_;
  std::string status_max_;
};

#endif // EVENT_PROJECTION_PARSER_H

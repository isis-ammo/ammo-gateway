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

#ifndef CHAT_PROJECTION_PARSER_H
#define CHAT_PROJECTION_PARSER_H

#include <string>

#include "StringParser.h"

class ChatProjectionParser : public StringParser
{
public:
  ChatProjectionParser (void);

  void parse (const std::string &params);

  std::string originator_;
  std::string group_id_;
  std::string text_;
  std::string latitude_min_;
  std::string latitude_max_;
  std::string longitude_min_;
  std::string longitude_max_;
  std::string uuid_;
  std::string pending_receipts_min_;
  std::string pending_receipts_max_;
  std::string media_count_min_;
  std::string media_count_max_;
  std::string status_min_;
  std::string status_max_;
  std::string created_date_min_;
  std::string created_date_max_;
  std::string modified_date_min_;
  std::string modified_date_max_;
};

#endif // CHAT_PROJECTION_PARSER_H



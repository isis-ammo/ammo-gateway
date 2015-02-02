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

#include "ChatProjectionParser.h"

ChatProjectionParser::ChatProjectionParser (void)
  : StringParser ()
{
}

void
ChatProjectionParser::parse (const std::string &params)
{
  tokenize (originator_, params);
  tokenize (group_id_, params);
  tokenize (text_, params);
  tokenize (latitude_min_, params);
  tokenize (latitude_max_, params);
  tokenize (longitude_min_, params);
  tokenize (longitude_max_, params);
  tokenize (uuid_, params);
  tokenize (pending_receipts_min_, params);
  tokenize (pending_receipts_max_, params);
  tokenize (media_count_min_, params);
  tokenize (media_count_max_, params);
  tokenize (status_min_, params);
  tokenize (status_max_, params);
  tokenize (created_date_min_, params);
  tokenize (created_date_max_, params);
  tokenize (modified_date_min_, params);
  tokenize (modified_date_max_, params);
  
  fini_check (params);
}


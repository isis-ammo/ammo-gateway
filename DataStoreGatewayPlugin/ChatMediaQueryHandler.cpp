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

#include "json/value.h"

#include "ChatMediaQueryHandler.h"
#include "ChatMediaProjectionParser.h"

ChatMediaQueryHandler::ChatMediaQueryHandler (
      sqlite3 *db,
      ammo::gateway::GatewayConnector *sender,
      ammo::gateway::PullRequest &pr)
  : OriginalQueryHandler (db, sender, pr)
{
}

bool
ChatMediaQueryHandler::matchedProjection (const Json::Value &root,
                                          const std::string &projection)
{
  ChatMediaProjectionParser parser;
  parser.parse (projection);
  
  return
    match_string (parser.uuid_, root["uuid"].asString ())
    && match_string (parser.messageId_, root["messageId"].asString ())
    && match_string (parser.name_, root["name"].asString ())
    && match_string (parser.data_, root["data"].asString ())
    && match_int (parser.createdDate_min_, root["createdDate"].asInt (), true)
    && match_int (parser.createdDate_max_, root["createdDate"].asInt (), false)
    && match_int (parser.receipts_min_, root["receipts"].asInt (), true)
    && match_int (parser.receipts_max_, root["receipts"].asInt (), false);
}



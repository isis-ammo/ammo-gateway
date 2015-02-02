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

#include "ChatQueryHandler.h"
#include "ChatProjectionParser.h"

ChatQueryHandler::ChatQueryHandler (
      sqlite3 *db,
      ammo::gateway::GatewayConnector *sender,
      ammo::gateway::PullRequest &pr)
  : OriginalQueryHandler (db, sender, pr)
{
}

bool
ChatQueryHandler::matchedProjection (const Json::Value &root,
                                     const std::string &projection)
{
  ChatProjectionParser parser;
  parser.parse (projection);
  
  return
    match_string (parser.originator_, root["originator"].asString ())
    && match_string (parser.group_id_, root["group_id"].asString ())
    && match_string (parser.text_, root["text"].asString ())
    && match_real (parser.latitude_min_, root["latitude"].asDouble (), true)
    && match_real (parser.latitude_max_, root["latitude"].asDouble (), false)
    && match_real (parser.longitude_min_, root["longitude"].asDouble (), true)
    && match_real (parser.longitude_max_, root["longitude"].asDouble (), false)
    && match_string (parser.uuid_, root["uuid"].asString ())
    && match_int (parser.pending_receipts_min_, root["pending_receipts"].asInt (), true)
    && match_int (parser.pending_receipts_max_, root["pending_receipts"].asInt (), false)
    && match_int (parser.media_count_min_, root["media_count"].asInt (), true)
    && match_int (parser.media_count_max_, root["media_count"].asInt (), false)
    && match_int (parser.status_min_, root["status"].asInt (), true)
    && match_int (parser.status_max_, root["status"].asInt (), false)
    && match_int (parser.created_date_min_, root["created_date"].asInt (), true)
    && match_int (parser.created_date_max_, root["created_date"].asInt (), false)
    && match_int (parser.modified_date_min_, root["modified_date"].asInt (), true)
    && match_int (parser.modified_date_max_, root["modified_date"].asInt (), false);
}



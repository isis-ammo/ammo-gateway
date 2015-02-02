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

#include "EventQueryHandler.h"
#include "EventProjectionParser.h"

EventQueryHandler::EventQueryHandler (
      sqlite3 *db,
      ammo::gateway::GatewayConnector *sender,
      ammo::gateway::PullRequest &pr)
  : OriginalQueryHandler (db, sender, pr)
{
}

bool
EventQueryHandler::matchedProjection (const Json::Value &root,
                                      const std::string &projection)
{
  EventProjectionParser parser;
  parser.parse (projection);
  
  return
    match_string (parser.uuid_, root["uuid"].asString ())
    && match_int (parser.mediaCount_min_, root["mediaCount"].asInt (), true)
    && match_int (parser.mediaCount_max_, root["mediaCount"].asInt (), false)
    && match_string (parser.displayName_, root["displayName"].asString ())
    && match_string (parser.categoryId_, root["categoryId"].asString ())
    && match_string (parser.title_, root["title"].asString ())
    && match_string (parser.description_, root["description"].asString ())
    && match_real (parser.longitude_min_, root["longitude"].asDouble (), true)
    && match_real (parser.longitude_max_, root["longitude"].asDouble (), false)
    && match_real (parser.latitude_min_, root["latitude"].asDouble (), true)
    && match_real (parser.latitude_max_, root["latitude"].asDouble (), false)
    && match_int (parser.createdDate_min_, root["createdDate"].asInt (), true)
    && match_int (parser.createdDate_max_, root["createdDate"].asInt (), false)
    && match_int (parser.modifiedDate_min_, root["modifiedDate"].asInt (), true)
    && match_string (parser.cid_, root["cid"].asString ())
    && match_string (parser.category_, root["category"].asString ())
    && match_string (parser.unit_, root["unit"].asString ())
    && match_int (parser.size_min_, root["size"].asInt (), true)
    && match_int (parser.size_max_, root["size"].asInt (), false)
    && match_string (parser.dest_group_name_, root["destGroupName"].asString ())
    && match_string (parser.dest_group_type_, root["destGroupType"].asString ())
    && match_int (parser.status_min_, root["status"].asInt (), true)
    && match_int (parser.status_max_, root["status"].asInt (), false);
}



/* Copyright (C) 2010-2013 Vanderbilt University
 *  
 * This software was developed by the Institute for Software Integrated
 * Systems (ISIS) at Vanderbilt University, Tennessee, USA for the
 * Transformative Apps program under DARPA, Contract # HR011-10-C-0175 and
 * Contract # HR0011-12-C-0109.
 *  
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this data, including any software or models in source or binary
 * form, as well as any drawings, specifications, and documentation
 * (collectively "the Data"), to deal in the Data without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Data, and to
 * permit persons to whom the Data is furnished to do so, subject to the
 * following conditions:
 *  
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Data.
 *  
 * THE DATA IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS, SPONSORS, DEVELOPERS, CONTRIBUTORS, OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE DATA OR THE USE OR OTHER DEALINGS IN THE DATA.
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



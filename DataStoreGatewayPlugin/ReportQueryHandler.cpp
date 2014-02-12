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

#include "ReportQueryHandler.h"
#include "ReportProjectionParser.h"

ReportQueryHandler::ReportQueryHandler (
      sqlite3 *db,
      ammo::gateway::GatewayConnector *sender,
      ammo::gateway::PullRequest &pr)
  : OriginalQueryHandler (db, sender, pr)
{
}

bool
ReportQueryHandler::matchedProjection (const Json::Value &root,
                                       const std::string &projection)
{
  ReportProjectionParser parser;
  parser.parse (projection);
  
  return
    match_string (parser.contentGuid_, root["contentGuid"].asString ())
    && match_int (parser.reportTime_min_, root["reportTime"].asInt (), true)
    && match_int (parser.reportTime_max_, root["reportTime"].asInt (),false)
    && match_string (parser.reportingUnit_, root["reportingUnit"].asString ())
    && match_int (parser.size_min_, root["size"].asInt (), true)
    && match_int (parser.size_max_, root["size"].asInt (), false)
    && match_string (parser.activity_, root["activity"].asString ())
    && match_string (parser.locationUtm_, root["locationUtm"].asString ())
    && match_string (parser.enemyUnit_, root["enemyUnit"].asString ())
    && match_int (parser.observationTime_min_, root["observationTime"].asInt (), true)
    && match_int (parser.observationTime_max_, root["observationTime"].asInt (), false)
    && match_string (parser.equipment_, root["equipment"].asString ())
    && match_string (parser.assessment_, root["assessment"].asString ())
    && match_string (parser.narrative_, root["narrative"].asString ())
    && match_string (parser.authentication_, root["authentication"].asString ());
}



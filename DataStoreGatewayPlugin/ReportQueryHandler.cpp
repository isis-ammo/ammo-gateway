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



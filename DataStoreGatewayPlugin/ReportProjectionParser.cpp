#include "ReportProjectionParser.h"

ReportProjectionParser::ReportProjectionParser (void)
  : StringParser ()
{
}

void
ReportProjectionParser::parse (const std::string &params)
{
  tokenize (contentGuid_, params);
  tokenize (reportTime_min_, params);
  tokenize (reportTime_max_, params);
  tokenize (reportingUnit_, params);
  tokenize (size_min_, params);
  tokenize (size_max_, params);
  tokenize (activity_, params);
  tokenize (locationUtm_, params);
  tokenize (enemyUnit_, params);
  tokenize (observationTime_min_, params);
  tokenize (observationTime_max_, params);
  tokenize (equipment_, params);
  tokenize (assessment_, params);
  tokenize (narrative_, params);
  tokenize (authentication_, params);
  
  fini_check (params);
}



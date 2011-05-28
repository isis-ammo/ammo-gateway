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

const std::string &
ReportProjectionParser::contentGuid (void) const
{
  return contentGuid_;
}

const std::string &
ReportProjectionParser::reportTime_min (void) const
{
  return reportTime_min_;
}

const std::string &
ReportProjectionParser::reportTime_max (void) const
{
  return reportTime_max_;
}

const std::string &
ReportProjectionParser::reportingUnit (void) const
{
  return reportingUnit_;
}

const std::string &
ReportProjectionParser::size_min (void) const
{
  return size_min_;
}

const std::string &
ReportProjectionParser::size_max (void) const
{
  return size_max_;
}

const std::string &
ReportProjectionParser::activity (void) const
{
  return activity_;
}

const std::string &
ReportProjectionParser::locationUtm (void) const
{
  return locationUtm_;
}
const std::string &
ReportProjectionParser::enemyUnit (void) const
{
  return enemyUnit_;
}
const std::string &
ReportProjectionParser::observationTime_min (void) const
{
  return observationTime_min_;
}
const std::string &
ReportProjectionParser::observationTime_max (void) const
{
  return observationTime_max_;
}
const std::string &
ReportProjectionParser::equipment (void) const
{
  return equipment_;
}
const std::string &
ReportProjectionParser::narrative (void) const
{
  return narrative_;
}
const std::string &
ReportProjectionParser::assessment (void) const
{
  return assessment_;
}
const std::string &
ReportProjectionParser::authentication (void) const
{
  return authentication_;
}


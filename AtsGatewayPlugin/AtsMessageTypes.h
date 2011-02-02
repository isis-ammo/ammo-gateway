#ifndef ATS_MESSAGE_TYPES_H
#define ATS_MESSAGE_TYPES_H

#include <string>

const std::string ATS_MSG_SPACE = "urn:aterrasys.com:/api/";

/**
List all people known to the system. Note that every user does not have an associated
person entry. Only users that are intended to map to a physical person (and therefore have
a person entry i nthe DB) will be returned by this call.
*/
const std::string RTC_PEOPLE_LIST = "rtc/people/list";

/** 
Create a new channel for real time collaboration
*/
const std::string RTC_CHANNEL_CREATE = "rtc/channel/create";

/**
Return a list of available channels.
*/
const std::string RTC_CHANNEL_LIST = "rtc/channel/list";

/**
Mark the channel as active. Channel start out active.
*/
const std::string RTC_CHANNEL_ACTIVATE = "rtc/channel/activate";


#endif // define ATS_MESSAGE_TYPES_H


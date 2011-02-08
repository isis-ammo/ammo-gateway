#ifndef ATS_MESSAGE_TYPES_H
#define ATS_MESSAGE_TYPES_H

#include <string>

const std::string ATS_NAME_SPACE = "urn:aterrasys.com:/api/";

/**
List all people known to the system. 
Note that every user does not have an associated person entry. 
Only users that are intended to map to a physical person 
(and therefore have a person entry in the DB) 
will be returned by this call.
*/
const std::string RTC_PEOPLE_LIST = "rtc/people/list";
const std::string RTC_PEOPLE_LIST_NS = ATS_NAME_SPACE+"rtc/people/list";

/** 
Create a new channel for real time collaboration
*/
const std::string RTC_CHANNEL_CREATE = "rtc/channel/create";
const std::string RTC_CHANNEL_CREATE_NS = ATS_NAME_SPACE+"rtc/channel/create";

/**
Return a list of available channels.
*/
const std::string RTC_CHANNEL_LIST = "rtc/channel/list";
const std::string RTC_CHANNEL_LIST_NS = ATS_NAME_SPACE+"rtc/channel/list";

/**
Mark the channel as active. Channel start out active.
*/
const std::string RTC_CHANNEL_ACTIVATE = "rtc/channel/activate";
const std::string RTC_CHANNEL_ACTIVATE_NS = ATS_NAME_SPACE+"rtc/channel/activate";

/**
Mark the channel as active. Channel start out active.
*/
const std::string RTC_CHANNEL_PASSIVATE = "rtc/channel/passivate";
const std::string RTC_CHANNEL_PASSIVATE_NS = ATS_NAME_SPACE+"rtc/channel/passivate";

/**
Upload a media file to the specified channel, with the specified metadata, 
and an optional clip file.

Mark the channel as active. A new channel starts out active.
*/
const std::string RTC_CHANNEL_MEDIA_UPLOAD = "media/upload";
const std::string RTC_CHANNEL_MEDIA_UPLOAD_NS = ATS_NAME_SPACE+"media/upload";

/**
List media available on the specified channel.
*/
const std::string RTC_CHANNEL_MEDIA_LIST = "media/list";
const std::string RTC_CHANNEL_MEDIA_LIST_NS = ATS_NAME_SPACE+"media/list";


/**
Return the specified media file.
*/
const std::string RTC_CHANNEL_MEDIA_FILE = "media/file";
const std::string RTC_CHANNEL_MEDIA_FILE_NS = ATS_NAME_SPACE+"media/file";

/**
Return the specified media clip.
Note: 
There is no difference enforced on the server end, 
but the idea is that the clip will be a reduced size version of the full file, 
so the server can hand back smaller files if requested by a client. 
How the reduced size version is created is client specific.
*/
const std::string RTC_CHANNEL_MEDIA_CLIP = "media/clip";
const std::string RTC_CHANNEL_MEDIA_CLIP_NS = ATS_NAME_SPACE+"media/clip";

/**
Return aggregate information since a given time. 
Currently returns channels and messages,
this can be changed to include more data as requested.
*/
const std::string RTC_CHANNEL_AGGREGATE = "rtc/aggregate";
const std::string RTC_CHANNEL_AGGREGATE_NS = ATS_NAME_SPACE+"rtc/aggregate";

// **  TEXT CHAT OPERATIONS

/**
Send text message invitations to users.
The user must have a valid SMS email gateway
configured in the database, and their mobile number must be known.
*/
const std::string RTC_INVITE = "chat/invite";
const std::string RTC_INVITE_NS = ATS_NAME_SPACE + RTC_INVITE;

/**
Post a GPS share message to an existing collaboration channel.
*/
const std::string RTC_SHARE_GPS = "chat/message/sharegps";
const std::string RTC_SHARE_GPS_NS = ATS_NAME_SPACE + RTC_SHARE_GPS;

#endif // define ATS_MESSAGE_TYPES_H


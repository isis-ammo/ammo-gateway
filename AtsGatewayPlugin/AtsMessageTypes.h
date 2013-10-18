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

#ifndef ATS_MESSAGE_TYPES_H
#define ATS_MESSAGE_TYPES_H

#include <string>

/**
 Every list has a corresponding created.
 The list is used primarily to initialize while created is used to keep up to date.
 list is returned on a pull while created is in response to a subscribe.
*/
const std::string ATS_NAME_SPACE = "urn:aterrasys.com:/api/";
const std::string TERMINATOR = "/";

/**
List all people known to the system. 
Note that every user does not have an associated person entry. 
Only users that are intended to map to a physical person 
(and therefore have a person entry in the DB) 
will be returned by this call.
*/
const std::string RTC_LIST_PEOPLE = "rtc/people/list";
const std::string RTC_LIST_PEOPLE_NS = ATS_NAME_SPACE+RTC_LIST_PEOPLE + TERMINATOR;

const std::string RTC_CREATE_PERSON = "rtc/people/create";
const std::string RTC_CREATE_PERSON_NS = ATS_NAME_SPACE+RTC_CREATE_PERSON + TERMINATOR;

const std::string RTC_PERSON_CREATED = "rtc/people/created";
const std::string RTC_PERSON_CREATED_NS = ATS_NAME_SPACE+RTC_PERSON_CREATED + TERMINATOR;

/** 
Create a new channel for real time collaboration
*/
const std::string RTC_CREATE_CHANNEL = "rtc/channel/create";
const std::string RTC_CREATE_CHANNEL_NS = ATS_NAME_SPACE+RTC_CREATE_CHANNEL + TERMINATOR;

const std::string RTC_CHANNEL_CREATED = "rtc/channel/created";
const std::string RTC_CHANNEL_CREATED_NS = ATS_NAME_SPACE+RTC_CHANNEL_CREATED;

/**
Return a list of available channels.
*/
const std::string RTC_LIST_CHANNEL = "rtc/channel/list";
const std::string RTC_LIST_CHANNEL_NS = ATS_NAME_SPACE+RTC_LIST_CHANNEL + TERMINATOR;

/**
Mark the channel as active. Channel start out active.
*/
const std::string RTC_ACTIVATE_CHANNEL = "rtc/channel/activate";
const std::string RTC_ACTIVATE_CHANNEL_NS = ATS_NAME_SPACE+RTC_ACTIVATE_CHANNEL + TERMINATOR;

const std::string RTC_CHANNEL_ACTIVATED = "rtc/channel/activated";
const std::string RTC_CHANNEL_ACTIVATED_NS = ATS_NAME_SPACE+RTC_CHANNEL_ACTIVATED + TERMINATOR;

/**
Mark the channel as active. Channel start out active.
*/
const std::string RTC_PASSIVATE_CHANNEL = "rtc/channel/passivate";
const std::string RTC_PASSIVATE_CHANNEL_NS = ATS_NAME_SPACE+RTC_PASSIVATE_CHANNEL + TERMINATOR;

const std::string RTC_CHANNEL_PASSIVATED = "rtc/channel/passivated";
const std::string RTC_CHANNEL_PASSIVATED_NS = ATS_NAME_SPACE+RTC_CHANNEL_PASSIVATED + TERMINATOR;

/**
List media available on the specified channel.
*/
const std::string RTC_LIST_CHANNEL_MEDIA = "media/list";
const std::string RTC_LIST_CHANNEL_MEDIA_NS = ATS_NAME_SPACE+RTC_LIST_CHANNEL_MEDIA + TERMINATOR;

/**
Upload a media file to the specified channel, with the specified metadata, 
and an optional clip file.

Mark the channel as active. A new channel starts out active.
*/
const std::string RTC_UPLOAD_CHANNEL_MEDIA = "media/upload";
const std::string RTC_UPLOAD_CHANNEL_MEDIA_NS = ATS_NAME_SPACE+RTC_UPLOAD_CHANNEL_MEDIA + TERMINATOR;

const std::string RTC_CHANNEL_MEDIA_UPLOADED = "media/uploaded";
const std::string RTC_CHANNEL_MEDIA_UPLOADED_NS = ATS_NAME_SPACE+RTC_CHANNEL_MEDIA_UPLOADED + TERMINATOR;

/**
A message indicating that a media was created.
*/
const std::string RTC_CHANNEL_MEDIA_CREATED = "media/created";
const std::string RTC_CHANNEL_MEDIA_CREATED_NS = ATS_NAME_SPACE+RTC_CHANNEL_MEDIA_CREATED + TERMINATOR;


/**
Pull the specified media file.
*/
const std::string RTC_GET_CHANNEL_MEDIA_FILE = "media/file";
const std::string RTC_GET_CHANNEL_MEDIA_FILE_NS = ATS_NAME_SPACE+ RTC_GET_CHANNEL_MEDIA_FILE + TERMINATOR;

/**
Pull the specified media clip.
Note: 
There is no difference enforced on the server end, 
but the idea is that the clip will be a reduced size version of the full file, 
so the server can hand back smaller files if requested by a client. 
How the reduced size version is created is client specific.
*/
const std::string RTC_GET_CHANNEL_MEDIA_CLIP = "media/clip";
const std::string RTC_GET_CHANNEL_MEDIA_CLIP_NS = ATS_NAME_SPACE+ RTC_GET_CHANNEL_MEDIA_CLIP + TERMINATOR;

/**
Pull aggregate information since a given time. 
Currently returns channels and messages,
this can be changed to include more data as requested.
*/
const std::string RTC_AGGREGATE_CHANNEL = "rtc/aggregate";
const std::string RTC_AGGREGATE_CHANNEL_NS = ATS_NAME_SPACE+ RTC_AGGREGATE_CHANNEL + TERMINATOR;

// ***************  TEXT CHAT OPERATIONS *******************

/**
Send text message invitations to users.
The user must have a valid SMS email gateway
configured in the database, and their mobile number must be known.
*/
const std::string RTC_INVITE = "chat/invite";
const std::string RTC_INVITE_NS = ATS_NAME_SPACE + RTC_INVITE + TERMINATOR;

/**
Post a GPS share message to an existing collaboration channel.
*/
const std::string RTC_SHARE_GPS = "chat/message/sharegps";
const std::string RTC_SHARE_GPS_NS = ATS_NAME_SPACE + RTC_SHARE_GPS + TERMINATOR;


/**
Post a canter map message to an existing collaboration channel.
*/
const std::string RTC_CENTER_MAP = "chat/message/centermap";
const std::string RTC_CENTER_MAP_NS = ATS_NAME_SPACE + RTC_CENTER_MAP + TERMINATOR;

/**
Post a textual message to an existing collaboration channel.
*/
const std::string RTC_POST_MSG = "chat/message/post";
const std::string RTC_POST_MSG_NS = ATS_NAME_SPACE + RTC_POST_MSG + TERMINATOR;

const std::string RTC_MSG_POSTED = "chat/message/posted";
const std::string RTC_MSG_POSTED_NS = ATS_NAME_SPACE + RTC_MSG_POSTED + TERMINATOR;

/**
Get messages on a channel since a specified time.
If no channel is specified, then all message since the
specified time for any channel to which the user is 
subscribed will be returned.
*/
const std::string RTC_GET_MSG = "chat/message/get";
const std::string RTC_GET_MSG_NS = ATS_NAME_SPACE + RTC_GET_MSG + TERMINATOR;

/**
Join a channel.
*/
const std::string RTC_JOIN = "chat/message/join";
const std::string RTC_JOIN_NS = ATS_NAME_SPACE + RTC_JOIN + TERMINATOR;

/**
Leave a channel.
*/
const std::string RTC_LEAVE = "chat/message/leave";
const std::string RTC_LEAVE_NS = ATS_NAME_SPACE + RTC_LEAVE + TERMINATOR;

/**
List all people who are members of the channel.
*/
const std::string RTC_LIST_MEMBERS = "people/list";
const std::string RTC_LIST_MEMBERS_NS = ATS_NAME_SPACE + RTC_LIST_MEMBERS + TERMINATOR;

// ***************  COLLABORATIVE DRAWING OPERATIONS *******************

/**
Create a new canvas in the specified channel. 
If the specified canvas name is already being
used in the specified channel, and error is returned, 
but the data segment of the response
will contain the id of the existing canvas.
*/
const std::string DRAW_CREATE_CANVAS = "whiteboard/canvas/create";
const std::string DRAW_CREATE_CANVAS_NS = ATS_NAME_SPACE + DRAW_CREATE_CANVAS + TERMINATOR;

/**
Set the current canvas for a given channel.
*/
const std::string DRAW_SET_CANVAS = "rtc/channel/setCanvas";
const std::string DRAW_SET_CANVAS_NS = ATS_NAME_SPACE + DRAW_SET_CANVAS + TERMINATOR;

/**
Set an existing media item as the background of this canvas.
*/
const std::string DRAW_BACKGROUND = "whiteboard/canvas/background";
const std::string DRAW_BACKGROUND_NS = ATS_NAME_SPACE + DRAW_BACKGROUND + TERMINATOR;

/**
Create a new path on the specified canvas, defined by the specified points.
*/
const std::string DRAW_CREATE_PATH = "whiteboard/path/create";
const std::string DRAW_CREATE_PATH_NS = ATS_NAME_SPACE + DRAW_CREATE_PATH + TERMINATOR;

const std::string DRAW_PATH_CREATED = "whiteboard/path/created";
const std::string DRAW_PATH_CREATED_NS = ATS_NAME_SPACE + DRAW_PATH_CREATED + TERMINATOR;

/**
List all paths in the specified canvas that were created after the specified time.
*/
const std::string DRAW_LIST_PATH = "whiteboard/path/list";
const std::string DRAW_LIST_PATH_NS = ATS_NAME_SPACE + DRAW_LIST_PATH + TERMINATOR;

/**
Delete the specified path.
*/
const std::string DRAW_DELETE_PATH = "whiteboard/path/delete";
const std::string DRAW_DELETE_PATH_NS = ATS_NAME_SPACE + DRAW_DELETE_PATH + TERMINATOR;

/**
Return the specified canvas, along with all paths and points that have been drawn to it.
*/
const std::string DRAW_GET_CANVAS = "whiteboard/canvas";
const std::string DRAW_GET_CANVAS_NS = ATS_NAME_SPACE + DRAW_GET_CANVAS + TERMINATOR;

/**
Remove all paths from the specified canvas, but do not delete the canvas itself;
it can continue to be used for drawing operations.
*/
const std::string DRAW_CLEAR = "whiteboard/canvas/clear";
const std::string DRAW_CLEAR_NS = ATS_NAME_SPACE + DRAW_CLEAR + TERMINATOR;

/**
Create text on a canvas
*/
const std::string DRAW_CREATE_TEXT = "whiteboard/text/create";
const std::string DRAW_CREATE_TEXT_NS = ATS_NAME_SPACE + DRAW_CREATE_TEXT + TERMINATOR;

/**
Lists text on a canvas created since a specified time.
*/
const std::string DRAW_LIST_TEXT = "whiteboard/text/list";
const std::string DRAW_LIST_TEXT_NS = ATS_NAME_SPACE + DRAW_LIST_TEXT + TERMINATOR;

/**
Delete text from a canvas.
*/
const std::string DRAW_DELETE_TEXT = "whiteboard/text/delete";
const std::string DRAW_DELETE_TEXT_NS = ATS_NAME_SPACE + DRAW_DELETE_TEXT + TERMINATOR;

// ***************  LOCATION TRACKING OPERATIONS *******************

/**
Update this user's position on the server, supplied in E6 notation
*/
const std::string PLI_POST_LOC = "location/post";
const std::string PLI_POST_LOC_NS = ATS_NAME_SPACE + PLI_POST_LOC + TERMINATOR;

/**
Update a group of user's position on the server, supplied in E6 notation
*/
const std::string PLI_POST_LOCS = "locations/post";
const std::string PLI_POST_LOCS_NS = ATS_NAME_SPACE + PLI_POST_LOCS + TERMINATOR;


/**
List all personnel and their locations for the specified unit.
*/
const std::string PLI_LIST_LOC = "location/list";
const std::string PLI_LIST_LOC_NS = ATS_NAME_SPACE + PLI_LIST_LOC + TERMINATOR;

/**
Create a new unit.
*/
const std::string PLI_CREATE_UNIT = "unit/create";
const std::string PLI_CREATE_UNIT_NS = ATS_NAME_SPACE + PLI_CREATE_UNIT + TERMINATOR;

const std::string PLI_CREATED_UNIT = "unit/created";
const std::string PLI_CREATED_UNIT_NS = ATS_NAME_SPACE + PLI_CREATED_UNIT + TERMINATOR;

/**
List all units known to the server.
*/
const std::string PLI_LIST_UNIT = "unit/list";
const std::string PLI_LIST_UNIT_NS = ATS_NAME_SPACE + PLI_LIST_UNIT + TERMINATOR;

/**
Join the specified unit. 
A user can join more than one unit as far as the API is concerned,
though operating procedure may prevent this in the field.
*/
const std::string PLI_JOIN_UNIT = "unit/join";
const std::string PLI_JOIN_UNIT_NS = ATS_NAME_SPACE + PLI_JOIN_UNIT + TERMINATOR;

/**
Leave the specified unit.
*/
const std::string PLI_LEAVE_UNIT = "unit/leave";
const std::string PLI_LEAVE_UNIT_NS = ATS_NAME_SPACE + PLI_LEAVE_UNIT + TERMINATOR;

/**
List members of a unit.
*/
const std::string PLI_MEMBERS = "unit/members";
const std::string PLI_MEMBERS_NS = ATS_NAME_SPACE + PLI_MEMBERS + TERMINATOR;

/**
List all users that fall in the box specified.
*/
const std::string PLI_FIND_IN_BOX = "unit/findInBox";
const std::string PLI_FIND_IN_BOX_NS = ATS_NAME_SPACE + PLI_FIND_IN_BOX + TERMINATOR;

// ***************  MAP OPERATIONS *******************

/**
Create an annotation on the map that contains optional text and/or media, 
and is geolocated at the specified lat/lon/zoom.
*/
const std::string MAP_CREATE_NOTE = "annotation/create";
const std::string MAP_CREATE_NOTE_NS = ATS_NAME_SPACE + MAP_CREATE_NOTE + TERMINATOR;


#endif // define ATS_MESSAGE_TYPES_H


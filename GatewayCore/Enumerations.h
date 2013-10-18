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

#ifndef ENUMERATIONS_H
#define ENUMERATIONS_H

enum MessageScope {
  SCOPE_GLOBAL = 0,
  SCOPE_LOCAL = 1,
  SCOPE_ALL = 2 //used by unregisterPullInterest, to remove a subscription regardless of scope
};

enum MessagePriority {
  PRIORITY_AUTH = 127,
  PRIORITY_CTRL = 112,
  PRIORITY_FLASH = 96,
  PRIORITY_URGENT = 64,
  PRIORITY_IMPORTANT = 32,
  PRIORITY_NORMAL = 0,
  PRIORITY_BACKGROUND = -32
};

enum PushStatus {
  PUSH_RECEIVED  = 0,
  PUSH_SUCCESS   = 1,
  PUSH_FAIL      = 2,
  PUSH_REJECTED  = 3
};

#endif //ENUMERATIONS_H

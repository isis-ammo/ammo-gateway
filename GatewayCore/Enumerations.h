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

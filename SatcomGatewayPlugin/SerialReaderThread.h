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

#ifndef SERIAL_READER_THREAD_H
#define SERIAL_READER_THREAD_H

#include <ace/Task.h>
#include <ace/Copy_Disabled.h>
#include <vector>
#include <iostream>

#include "Typedefs.h"

class SerialConnector;
struct SatcomHeader;

class SerialReaderThread : public ACE_Task<ACE_MT_SYNCH>, public ACE_Copy_Disabled {
public:
  SerialReaderThread(SerialConnector *connector);
  virtual ~SerialReaderThread();

  virtual int svc();

  void stop();

private:
  enum ReaderThreadState {
    STATE_READING_MAGIC,
    STATE_READING_HEADER,
    STATE_READING_DATA
  };

  bool validateHeaderChecksum(const SatcomHeader &header);

  bool readAChar(uint8_t &result);

  bool processData(const SatcomHeader &header, const std::string &payload);

  SerialConnector *connector;

  ACE_Thread_Mutex closeMutex;
  bool closed;
  bool isClosed();

  friend std::ostream& operator<<( std::ostream& stream, const SerialReaderThread::ReaderThreadState &state );
};

inline std::ostream& operator<<( std::ostream& stream, const SerialReaderThread::ReaderThreadState &state )
{
    switch(state) {
    case SerialReaderThread::STATE_READING_MAGIC:
      stream << "Reading magic";
      break;
    case SerialReaderThread::STATE_READING_HEADER:
      stream << "Reading header";
      break;
    case SerialReaderThread::STATE_READING_DATA:
      stream << "Reading data";
      break;
    default:
      stream << "Unknown state (" << state << ")";
      break;
    }
    return stream;
}

#endif //SERIAL_READER_THREAD_H

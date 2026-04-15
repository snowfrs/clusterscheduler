/*___INFO__MARK_BEGIN__*/
/*************************************************************************
 * 
 *  The Contents of this file are made available subject to the terms of
 *  the Sun Industry Standards Source License Version 1.2
 * 
 *  Sun Microsystems Inc., March, 2001
 * 
 * 
 *  Sun Industry Standards Source License Version 1.2
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.2 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://gridengine.sunsource.net/Gridengine_SISSL_license.html
 * 
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 * 
 *   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 * 
 *   Copyright: 2001 by Sun Microsystems, Inc.
 * 
 *   All Rights Reserved.
 * 
 *  Portions of this software are Copyright (c) 2023-2024,2026 HPC-Gridware GmbH
 *
 ************************************************************************/
/*___INFO__MARK_END__*/

#include <ctime>
#include <chrono>
#include <sys/times.h>

#include <sys/time.h>

#include "uti/sge_dstring.h"
#include "uti/sge_string.h"
#include "uti/sge_time.h"

uint64_t sge_get_gmt64() {
   const auto now = std::chrono::system_clock::now();
   const auto epoch = now.time_since_epoch();
   const auto us = duration_cast<std::chrono::microseconds>(epoch);
   return us.count();
}

uint32_t sge_gmt64_to_gmt32(uint64_t timestamp) {
   uint64_t ret = timestamp / 1000000;
   if (ret > std::numeric_limits<uint32_t>::max()) {
      ret = std::numeric_limits<uint32_t>::max();
   }
   return (uint32_t)ret;
}

time_t sge_gmt64_to_time_t(uint64_t timestamp) {
   uint64_t ret = timestamp / 1000000;
   return (time_t)ret;
}

double sge_gmt64_to_gmt32_double(uint64_t timestamp) {
   auto ret = (double)timestamp;
   return ret / 1000000.0;
}

uint64_t sge_time_t_to_gmt64(time_t timestamp) {
   uint64_t ret = timestamp;
   return ret * 1000000;
}

void sge_gmt64_to_timespec(uint64_t timestamp, struct timespec &ts) {
   ts.tv_sec = (time_t)(timestamp / 1000000);
   ts.tv_nsec = (long)(timestamp % 1000000) * 1000;
}

const char *append_time(uint64_t timestamp, dstring *dstr, bool is_xml) {
   DSTRING_STATIC(local_dstr, 100);
   return sge_dstring_append(dstr, sge_ctime64(timestamp, &local_dstr, is_xml, true));
}

/****** uti/time/append_time() **************************************************
*  NAME
*     append_time() -- Convert time value into string 
*
*  SYNOPSIS
*     const char* append_time(time_t i, dstring *buffer) 
*
*  FUNCTION
*     Convert time value into string 
*
*  INPUTS
*     time_t i - time value 
*     dstring *buffer - dstring
*     bool is_xml - write in XML dateTime format?
*
*  RESULT
*     const char* - time string (current time if 'i' was 0) 
*     dstring *buffer - buffer provided by caller
*
*  NOTES
*     MT-NOTE: append_time() is MT safe if localtime_r() can be used
******************************************************************************/
const char *append_time(time_t i, dstring *buffer, bool is_xml) {
   const char *ret;
   struct tm tm_buffer{};
   auto *tm = (struct tm *) localtime_r(&i, &tm_buffer);

   if (is_xml) {
      ret = sge_dstring_sprintf_append(buffer, "%04d-%02d-%02dT%02d:%02d:%02d",
                                 1900 + tm->tm_year, tm->tm_mon + 1, tm->tm_mday,
                                 tm->tm_hour, tm->tm_min, tm->tm_sec);
   } else {
      ret = sge_dstring_sprintf_append(buffer, "%02d/%02d/%04d %02d:%02d:%02d",
                                 tm->tm_mon + 1, tm->tm_mday, 1900 + tm->tm_year,
                                 tm->tm_hour, tm->tm_min, tm->tm_sec);
   }

   return ret;
}

const char *sge_ctime64(uint64_t timestamp, dstring *dstr, bool is_xml, bool with_micro) {
   const char *ret;

   if (timestamp == 0) {
      timestamp = sge_get_gmt64();
   }

   const std::chrono::microseconds us{timestamp};
   const std::chrono::seconds s = duration_cast<std::chrono::seconds>(us);
   time_t t = (time_t)s.count();
   struct tm tm{};

   if (localtime_r(&t, &tm) == nullptr) {
      ret = sge_strerror(errno, dstr);
   } else {
      // we could call the 32bit version of append_time here
      if (is_xml) {
         ret = sge_dstring_sprintf(dstr, "%04d-%02d-%02dT%02d:%02d:%02d",
                                   1900 + tm.tm_year, tm.tm_mon + 1, tm.tm_mday,
                                   tm.tm_hour, tm.tm_min, tm.tm_sec);
      } else {
         ret = sge_dstring_sprintf(dstr, "%04d-%02d-%02d %02d:%02d:%02d",
                                   1900 + tm.tm_year, tm.tm_mon + 1, tm.tm_mday,
                                   tm.tm_hour, tm.tm_min, tm.tm_sec);
      }
      if (with_micro) {
         long micro = us.count() % 1000000;
         ret = sge_dstring_sprintf_append(dstr, ".%06ld", micro);
      }
   }

   return ret;
}

const char *sge_ctime64(uint64_t timestamp, dstring *dstr) {
   return sge_ctime64(timestamp, dstr, false, true);
}

const char *sge_ctime64_short(uint64_t timestamp, dstring *dstr) {
   return sge_ctime64(timestamp, dstr, false, false);
}

const char *sge_ctime64_xml(uint64_t timestamp, dstring *dstr) {
   return sge_ctime64(timestamp, dstr, true, true);
}

const char *sge_ctime64_date_time(uint64_t timestamp, dstring *dstr) {
   const char *ret;

   if (timestamp == 0) {
      timestamp = sge_get_gmt64();
   }

   const std::chrono::microseconds us{timestamp};
   const std::chrono::seconds s = duration_cast<std::chrono::seconds>(us);
   time_t t = (time_t)s.count();
   struct tm tm{};

   if (localtime_r(&t, &tm) == nullptr) {
      ret = sge_strerror(errno, dstr);
   } else {
      ret = sge_dstring_sprintf(dstr, "%04d%02d%02d%02d%02d.%02d",
                                1900 + tm.tm_year, tm.tm_mon + 1, tm.tm_mday,
                                tm.tm_hour, tm.tm_min, tm.tm_sec);
   }

   return ret;
}

const char *sge_at_time(time_t i, dstring *buffer) {
   struct tm tm_buffer{};

   if (!i)
      i = time(nullptr);
   auto *tm = (struct tm *) localtime_r(&i, &tm_buffer);
   return sge_dstring_sprintf(buffer, "%04d%02d%02d%02d%02d.%02d",
                              tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                              tm->tm_hour, tm->tm_min, tm->tm_sec);
}

/****** uti/time/duration_add_offset() ****************************************
*  NAME
*     duration_add_offset() -- add function for time add
*
*  SYNOPSIS
*     uint32_t duration_add_offset(uint32_t duration, uint32_t offset)
*
*  FUNCTION
*     add function to catch ulong overflow. Returns max ulong value if necessary
*
*  INPUTS
*     uint32_t duration - duration in seconds
*     uint32_t offset   - offset in seconds
*
*  RESULT
*     uint32_t - value < std::numeric_limits<uint32_t>::max()
*
*  NOTES
*     MT-NOTE: duration_add_offset() is not MT safe 
*******************************************************************************/
uint64_t duration_add_offset(uint64_t duration, uint64_t offset) {
   if (duration == std::numeric_limits<uint64_t>::max() || offset == std::numeric_limits<uint64_t>::max()) {
      return std::numeric_limits<uint64_t>::max();
   }

   if ((std::numeric_limits<uint64_t>::max() - offset) < duration) {
      duration = std::numeric_limits<uint64_t>::max();
   } else {
      duration += offset;
   }

   return duration;
}

/****** uti/time/sge_usleep() ****************************************
*  NAME
*     sge_usleep() -- Mimiks a non-iterruptable usleep() 
*
*  SYNOPSIS
*     void sge_usleep(int sleep_time) 
*
*  FUNCTION
*     Mimiks a non-iterruptable usleep() to the caller.
*
*  INPUTS
*     int sleep_time - requested sleep time
*
*  RESULT
*     n/a
*
*  NOTES
*     None.
*******************************************************************************/
void sge_usleep(int sleep_time) {
   struct timeval wake_tv{}, sleep_tv{}, snooze_tv{};
   int time_to_sleep = sleep_time;

   do {
      gettimeofday(&sleep_tv, nullptr);
      usleep(time_to_sleep);
      gettimeofday(&wake_tv, nullptr);
      if (wake_tv.tv_usec < sleep_tv.tv_usec) {
         wake_tv.tv_sec--;
         wake_tv.tv_usec = wake_tv.tv_usec + 1000000;
      }
      snooze_tv.tv_sec = wake_tv.tv_sec - sleep_tv.tv_sec;
      snooze_tv.tv_usec = wake_tv.tv_usec - sleep_tv.tv_usec;

      time_to_sleep = time_to_sleep - (snooze_tv.tv_sec * 1000000 + snooze_tv.tv_usec);
   } while (time_to_sleep > 0);

   return;
}

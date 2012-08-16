/* Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#ifndef LOG4CXX_PRIVATE_LOG4CXX_H
#define LOG4CXX_PRIVATE_LOG4CXX_H

/* GENERATED FILE WARNING!  DO NOT EDIT log4cxx_private.h
 *
 * You must modify log4cxx_private.h.in instead.
 *
 */

#include <log4cxx/log4cxx.h>

#if !defined(LOG4CXX) && !defined(LOG4CXX_TEST)
#error "log4cxx/private/log4cxx_private.h should only be used within log4cxx implementation or tests"
#endif

#define LOG4CXX_RETURN_AFTER_THROW 0

#define LOG4CXX_HAS_STD_LOCALE 1
#define LOG4CXX_FORCE_WIDE_CONSOLE 0
#define LOG4CXX_FORCE_BYTE_CONSOLE 0
#ifdef WIN32
#  define LOG4CXX_MEMSET_IOS_BASE 1
#endif
#define LOG4CXX_HAVE_ODBC 0
#define LOG4CXX_HAS_MBSRTOWCS 1
#define LOG4CXX_HAS_WCSTOMBS 1
#define LOG4CXX_HAS_FWIDE 1

#define LOG4CXX_CHARSET_UTF8 0
#define LOG4CXX_CHARSET_ISO88591 0
#define LOG4CXX_CHARSET_USASCII 0
#define LOG4CXX_CHARSET_EBCDIC 0

#define LOG4CXX_HAVE_LIBESMTP 0
#define LOG4CXX_HAVE_SYSLOG 1

#define LOG4CXX_WIN32_THREAD_FMTSPEC "0x%.8x"
#define LOG4CXX_APR_THREAD_FMTSPEC "0x%pt"


#endif

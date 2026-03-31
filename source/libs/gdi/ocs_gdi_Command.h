#pragma once
/*___INFO__MARK_BEGIN_NEW__*/
/***************************************************************************
 *
 *  Copyright 2024-2025 HPC-Gridware GmbH
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ***************************************************************************/
/*___INFO__MARK_END_NEW__*/

#include <cstdint>
#include <string>

namespace ocs::gdi {
   enum class Command : __uint32_t {
      NONE = 0,
      GET,
      ADD,
      DEL,
      MOD,
      TRIGGER,
      PERMCHECK,
      SPECIAL,
      COPY,
      REPLACE,
      PROCEDURE,
   };

   std::string to_string(Command cmd);

   inline Command operator|(Command a, Command b) {
      return static_cast<Command>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
   }

   inline Command operator&(Command a, Command b) {
      return static_cast<Command>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
   }
}

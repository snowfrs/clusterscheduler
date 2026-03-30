#pragma once
/*___INFO__MARK_BEGIN_NEW__*/
/***************************************************************************
 *
 *  Copyright 2026 HPC-Gridware GmbH
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

#include "cull/cull.h"

namespace ocs {
   class QQuotaParameter {
   public:
      enum class OutputFormat{
         PLAIN,
         XML,
         JSON
      };

   private:
      OutputFormat output_format = OutputFormat::PLAIN;  //< -fmt output_format
      lList *host_list = nullptr;                        //< -h host_list
      lList *resource_match_list = nullptr;              //< -l resource_request
      lList *user_list = nullptr;                        //< -u user_list
      lList *pe_list = nullptr;                          //< -pe pe_list
      lList *project_list = nullptr;                     //< -P project_list
      lList *cqueue_list = nullptr;                      //< -q wc_queue_list

      void free_data();

      bool show_usage(FILE *fp);
      bool parse_cmdline_and_env(char **argv, lList **switch_list, lList **answer_list);
      bool parse_cmdline_from_file(const char *file, lList **switch_list, lList **answer_list);
      bool parse_switch_list(lList **switch_list, lList **answer_list);
   public:
      QQuotaParameter() = default;
      virtual ~QQuotaParameter() { free_data(); }

      [[nodiscard]] OutputFormat get_output_format() const { return output_format; }
      [[nodiscard]] lList *get_host_list() const { return host_list; }
      [[nodiscard]] lList *get_resource_match_list() const { return resource_match_list; }
      [[nodiscard]] lList *get_user_list() const { return user_list; }
      [[nodiscard]] lList *get_pe_list() const { return pe_list; }
      [[nodiscard]] lList *get_project_list() const { return project_list; }
      [[nodiscard]] lList *get_cqueue_list() const { return cqueue_list; }

      bool parse_parameters(lList **answer_list, char **argv, char **envp);
   };
}
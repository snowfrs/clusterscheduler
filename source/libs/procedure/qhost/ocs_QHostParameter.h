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

#include <string>

#include "cull/cull.h"

#include "ocs_ProcedureParameter.h"

namespace ocs {
   class QHostParameter : public ProcedureParameter {

#pragma region Data Types

   public:
      enum class OutputFormat : uint32_t {
         PLAIN,
         XML,
         JSON
      };

#pragma endregion

#pragma region Constants

   private:
      static constexpr auto HOSTNAME_LIST = "hostname_list";
      static constexpr auto USER_NAME_LIST = "user_name_list";
      static constexpr auto RESOURCE_LIST = "resource_list";
      static constexpr auto RESOURCE_VISIBLE_LIST = "resource_visible_list";
      static constexpr auto SHOW = "show";
      static constexpr auto OUTPUT_FORMAT = "output_format";
      static constexpr auto NAME_VALUE_LIST = "name_value_list";
      static constexpr auto PROCEDURE = "procedure";

#pragma endregion

#pragma region Data

   private:
      std::string procedure_name_ = prognames[QHOST];
      lList *hostname_list_ = nullptr;
      lList *user_name_list_ = nullptr;
      lList *resource_match_list_ = nullptr;
      lList *resource_visible_list_ = nullptr;
      uint32_t show_ = 0;
      OutputFormat output_format_ = OutputFormat::PLAIN;

   public:
      [[nodiscard]] const lList *get_hostname_list() const { return hostname_list_; }
      [[nodiscard]] const lList *get_user_name_list() const { return user_name_list_; }
      [[nodiscard]] const lList *get_resource_match_list() const { return resource_match_list_; }
      [[nodiscard]] const lList *get_resource_visible_list() const { return resource_visible_list_; }
      [[nodiscard]] uint32_t get_show() const { return show_; }
      [[nodiscard]] OutputFormat get_output_format() const { return output_format_ ; }

#pragma endregion

#pragma region Client Side Parsing

   private:
      static bool show_usage(FILE *fp);
      bool parse_cmdline_and_env(char **argv, char **env, lList **cmdline, lList **answer_list);
      bool parse_cmdline_from_file(lList **cmdline, lList **answer_list, const char *file);
      int parse_switch_list(lList **cmdline, lList **answer_list);

   public:
      bool parse_parameters(lList **answer_list, char **argv, char **envp);

#pragma endregion

#pragma region Constructor/Destructor
   private:
      void free_data();

   public:
      QHostParameter();
      ~QHostParameter() override { free_data(); }

#pragma endregion

#pragma region Marshaling

      [[nodiscard]] lList *get_bundle() override;
      void set_bundle(lList **bundle) override;

#pragma endregion
   };
}
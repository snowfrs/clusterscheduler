#pragma once
/*___INFO__MARK_BEGIN_NEW__*/
/***************************************************************************
 *
 *  Copyright 2023-2026 HPC-Gridware GmbH
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

namespace ocs {
   class ProcedureParameter {

#pragma region Data Types

   public:
      enum class OutputFormat : uint32_t {
         PLAIN,
         XML,
         JSON
      };

      enum class ExecContext : uint32_t {
         CLIENT,
         SERVER
      };

#pragma endregion

#pragma region Data

   protected:
      OutputFormat output_format_ = OutputFormat::PLAIN;
      ExecContext exec_context_ = ExecContext::SERVER;

   public:
      [[nodiscard]] OutputFormat get_output_format() const { return output_format_ ; }
      [[nodiscard]] ExecContext get_exec_context() const { return exec_context_ ; }
      //[[nodiscard]] const std::string& get_procedure_name() const { return procedure_name_; }

#pragma endregion

#pragma region Constants

   public:
      static constexpr auto NAME_VALUE_LIST = "name_value_list";
      static constexpr auto PROCEDURE = "procedure";
      static constexpr auto RESPONSE = "response";

#pragma endregion

#pragma region Marshaling

   protected:
      lList *parameter_bundle = nullptr; ///< SPP_Type

   public:
      [[nodiscard]] virtual lList *get_bundle(const std::string& procedure_name);
      virtual void set_bundle(lList **bundle);
      void add_parameter_bundle(const std::string& name, lList *parameter);
      static std::string get_procedure_from_bundle(const lList *parameter_bundle);

#pragma endregion

#pragma region Constructor/Destructor

   public:
      ProcedureParameter() = default;
      virtual ~ProcedureParameter() = default;

#pragma endregion
   };
}

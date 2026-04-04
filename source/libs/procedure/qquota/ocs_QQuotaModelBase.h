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

#include "ocs_QQuotaParameter.h"

namespace ocs {
   class QQuotaModelBase {
#pragma region Data
   protected:
      lList *centry_list_ = nullptr;
      lList *user_set_list_ = nullptr;
      lList *hgroup_list_ = nullptr;
      lList *exec_host_list_ = nullptr;
      lList *rqs_list_ = nullptr;

   public:
      [[nodiscard]] const lList *get_rqs_list() const { return rqs_list_; }
      [[nodiscard]] const lList *get_centry_list() const { return centry_list_; }
      [[nodiscard]] const lList *get_user_set_list() const { return user_set_list_; }
      [[nodiscard]] const lList *get_hgroup_list() const { return hgroup_list_; }
      [[nodiscard]] const lList *get_exec_host_list() const { return exec_host_list_; }
#pragma endregion

#pragma region Data Retrieval
   protected:
      static lEnumeration *get_rqs_what();
      static lEnumeration *get_centry_what();
      static lEnumeration *get_user_set_what();
      static lEnumeration *get_hgroup_what();
      static lEnumeration *get_host_what();
      static lCondition *get_host_where(const lList *host_list);

      virtual bool fetch_data(lList **answer_list, const lList *host_list);
   public:
      virtual bool make_snapshot(lList **answer_list, QQuotaParameter &parameter);
#pragma endregion

#pragma region Constructors/Destructors
   public:
      QQuotaModelBase() = default;
      virtual ~QQuotaModelBase();
#pragma endregion

   };
}

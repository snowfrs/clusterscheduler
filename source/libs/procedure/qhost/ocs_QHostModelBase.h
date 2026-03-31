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

#include "ocs_QHostParameter.h"


namespace ocs {

   /** @brief Base class for QHost data models.
    *
    * This class defines the interface for QHost data models and provides common functionality for managing the data
    * lists. Derived classes must implement the data retrieval, preparation, filtering, and sorting logic.
    *
    * The class also provides accessors for the data lists and a flag indicating whether the model is a manager model.
    * The destructor is virtual to ensure proper cleanup of resources in derived classes.
    */
   class QHostModelBase {

#pragma region Data

   protected:
      bool is_manager_ = false;        ///< True if the executing user has manager privileges
      lList *acl_list_ = nullptr;      ///< Access controll lists
      lList *centry_list_ = nullptr;   ///< Complex entry lists
      lList *config_list_ = nullptr;   ///< Configuration lists
      lList *exechost_list_ = nullptr; ///< Execution host lists
      lList *job_list_ = nullptr;      ///< Job lists
      lList *pe_list_ = nullptr;       ///< Parallel environment lists
      lList *queue_list_ = nullptr;    ///< Queue lists

   public:
      [[nodiscard]] virtual bool is_manager() const { return is_manager_; }
      [[nodiscard]] virtual lList *get_queue_list() const { return queue_list_; }
      [[nodiscard]] virtual lList *get_job_list() const { return job_list_; }
      [[nodiscard]] virtual lList *get_centry_list() const { return centry_list_; }
      [[nodiscard]] virtual lList *get_exechost_list() const { return exechost_list_; }
      [[nodiscard]] virtual lList *get_pe_list() const { return pe_list_; }
      [[nodiscard]] virtual lList *get_acl_list() const { return acl_list_; }

#pragma endregion

#pragma region Deta Retrieval

   private:

      virtual bool fetch_data(lList **answer_list, const lList *hostname_list, const lList *user_name_list, uint32_t show) = 0;
      virtual bool prepare_data(lList **answer_list, const lList *resource_match_list, uint32_t show) const = 0;
      virtual void filter_data(const lList *resource_match_list) = 0;
      virtual void sort_data() = 0;

   public:

      virtual bool make_snapshot(lList **answer_list, QHostParameter &parameter) = 0;

#pragma endregion

#pragma region Constructors/Destructors

   public:
      QHostModelBase() = default;
      virtual ~QHostModelBase();

#pragma endregion


   };
}

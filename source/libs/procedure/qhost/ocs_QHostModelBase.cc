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

#include "uti/ocs_Pattern.h"
#include "uti/sge_log.h"
#include "uti/sge_rmon_macros.h"
#include "uti/sge.h"

#include "sgeobj/sge_centry.h"
#include "sgeobj/sge_host.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_ja_task.h"
#include "sgeobj/sge_qinstance.h"
#include "sgeobj/sge_str.h"
#include "sgeobj/sge_pe.h"
#include "sgeobj/sge_userset.h"

#include "ocs_QHostModelBase.h"
#include "ocs_client_print.h"
#include "sched/sge_select_queue.h"
#include "uti/sge_hostname.h"


ocs::QHostModelBase::~QHostModelBase() {
   DENTER(TOP_LAYER);
   lFreeList(&acl_list_);
   lFreeList(&centry_list_);
   lFreeList(&config_list_);
   lFreeList(&exechost_list_);
   lFreeList(&job_list_);
   lFreeList(&pe_list_);
   lFreeList(&queue_list_);
   DRETURN_VOID;
}

void ocs::QHostModelBase::log_details() const {
   DENTER(TOP_LAYER);
   if (is_manager_) {
      DPRINTF("User has manager privileges\n");
   } else {
      DPRINTF("User does not have manager privileges\n");
   }
   DPRINTF("ACL list contains " sge_u32 " elements\n", lGetNumberOfElem(acl_list_));
   DPRINTF("centry list contains " sge_u32 " elements\n", lGetNumberOfElem(centry_list_));
   DPRINTF("ehost list contains " sge_u32 " elements\n", lGetNumberOfElem(exechost_list_));
   DPRINTF("job list contains " sge_u32 " elements\n", lGetNumberOfElem(job_list_));
   DPRINTF("PE list contains " sge_u32 " elements\n", lGetNumberOfElem(pe_list_));
   DPRINTF("cqueue list contains " sge_u32 " elements\n", lGetNumberOfElem(queue_list_));
   DPRINTF("conf list contains " sge_u32 " elements\n", lGetNumberOfElem(config_list_));
   DRETURN_VOID;
}

bool ocs::QHostModelBase::fetch_data(lList **answer_list, const lList *hostname_list, const lList *user_name_list, uint32_t show) {
   // Nothing common to do
   return true;
}

bool ocs::QHostModelBase::prepare_data(lList **answer_list, const lList *resource_match_list, uint32_t show) const {
   DENTER(TOP_LAYER);
   // tag jobs for printing if they should be shown
   if ((show & QHOST_DISPLAY_JOBS) == QHOST_DISPLAY_JOBS) {
      const lListElem *job = nullptr;
      for_each_ep (job, job_list_) {
         lListElem *ja_task;

         for_each_rw(ja_task, lGetList(job, JB_ja_tasks)) {
            lSetUlong(ja_task, JAT_suitable, TAG_SHOW_IT);
         }
      }
   }

   // init steps required only if there is a resource match request (-l)
   if (lGetNumberOfElem(resource_match_list)) {

      // prepare complex attribute list for matching
      centry_list_init_double(centry_list_);
      if (centry_list_fill_request(resource_match_list, answer_list, centry_list_, true, true, false)) {
         DRETURN(false);
      }

      // initialize tag field for hosts
      lListElem *host = nullptr;
      for_each_rw (host, exechost_list_) {
         lSetUlong(host, EH_tagged, 0);
      }
   }
   DRETURN(true);
}

void ocs::QHostModelBase::filter_data(const lList *resource_match_list) {
   DENTER(TOP_LAYER);

   // early exit if there is no resource match request
   if (lGetNumberOfElem(resource_match_list) == 0) {
      DRETURN_VOID;
   }

   // we will modify the resource_match_list, so we need to work on a copy of it
   lList *tmp_resource_list = lCopyList(nullptr, resource_match_list);
   lListElem *global = lGetElemHostRW(exechost_list_, EH_name, SGE_GLOBAL_NAME);
   bool is_selected = sge_select_queue(tmp_resource_list, nullptr, global, exechost_list_,
                                       centry_list_, true, -1, nullptr,
                                       nullptr, nullptr);
   lListElem *host;
   if (is_selected) {
      for_each_rw(host, exechost_list_) {
         lSetUlong(host, EH_tagged, 1);
      }
   } else {
      // If there is hostname request, remove it as we cannot match hostname in sge_select_queue()
      // We will process this tmp_resource_list separately!
      lListElem *host_match_elem = lGetElemStrRW(tmp_resource_list, CE_name, "hostname");
      if (host_match_elem != nullptr) {
         lDechainElem(tmp_resource_list, host_match_elem);
      }

      for_each_rw (host, exechost_list_) {
         const char *hostname = lGetHost(host, EH_name);

         // try to find a matching attribute
         is_selected = sge_select_queue(tmp_resource_list, nullptr, host, exechost_list_, centry_list_,
                                     true, -1, nullptr, nullptr, nullptr);
         if (is_selected) {
            // this host is selected, tag it for output
            lSetUlong(host, EH_tagged, 1);

            // if there is hostname request, check it here as sge_select_queue cannot handle it
            if (host_match_elem != nullptr && sge_hostcmp(lGetString(host_match_elem, CE_stringval), hostname) != 0 ) {
               lSetUlong(host, EH_tagged, 0);
            }
         } else {
            lSetUlong(host, EH_tagged, 0);
         }
      }
      lFreeList(&tmp_resource_list);
      if (host_match_elem != nullptr) {
         lFreeElem(&host_match_elem);
      }
   }

   // Reduce the hostlist, only the tagged ones survive
   lCondition *where = lWhere("%T(%I == %u)", EH_Type, EH_tagged, 1);
   lSplit(&exechost_list_, nullptr, nullptr, where);
   lFreeWhere(&where);

   DRETURN_VOID;
}

void ocs::QHostModelBase::sort_data() {
   DENTER(TOP_LAYER);

   // Find and de-chain the global host
   lListElem *global_host = lGetElemHostRW(exechost_list_, EH_name, SGE_GLOBAL_NAME);
   lDechainElem(exechost_list_, global_host);

   // Sort remaining elements alphabetically
   lPSortList(exechost_list_, "%I+", EH_name);

   // Re-insert global host at the beginning
   if (global_host != nullptr) {
      lInsertElem(exechost_list_, nullptr, global_host);
   }

   DRETURN_VOID;
}

bool
ocs::QHostModelBase::make_snapshot(lList **answer_list, QHostParameter &parameter) {
   DENTER(TOP_LAYER);
   const lList *hostname_list = parameter.get_hostname_list();
   const lList *user_name_list = parameter.get_user_name_list();
   const lList *resource_match_list = parameter.get_resource_match_list();
   const uint32_t show = parameter.get_show();

   // fetch data from qmaster
   if (!fetch_data(answer_list, hostname_list, user_name_list, show)) {
      DRETURN(false);
   }

   // init raw data received from qmaster
   if (!prepare_data(answer_list, resource_match_list, show)) {
      DRETURN(false);
   }

   // filter and sort data for output
   filter_data(resource_match_list);
   sort_data();

   DRETURN(true);
}

/** @brief Returns a hostname filter.
 */
lCondition *
ocs::QHostModelBase::get_host_where(const lList *hostname_list) {
   DENTER(TOP_LAYER);
   lCondition *where= nullptr;

   const lListElem *host;
   for_each_ep(host, hostname_list) {
      lCondition *new_where = lWhere("%T(%I h= %s)", EH_Type, EH_name, lGetString(host, ST_name));
      if (!where) {
         where = new_where;
      } else {
         where = lOrWhere(where, new_where);
      }
   }

   // without host request from the user we include the global host, too.
   if (where != nullptr) {
      const lCondition *new_where = lWhere("%T(%I == %s)", EH_Type, EH_name, SGE_GLOBAL_NAME);
      where = lOrWhere(where, new_where);
   }

   // But we do not want the template host
   lCondition *new_where = lWhere("%T(%I != %s)", EH_Type, EH_name, SGE_TEMPLATE_NAME);
   if (where != nullptr) {
      where = lAndWhere(where, new_where);
   } else {
      where = new_where;
   }

   DRETURN(where);
}

lEnumeration *ocs::QHostModelBase::get_host_what() {
   return lWhat("%T(ALL)", EH_Type);
}

lEnumeration *ocs::QHostModelBase::get_queue_what() {
   return lWhat("%T(ALL)", QU_Type);
}

lCondition * ocs::QHostModelBase::get_job_where(const lList *user_name_list, const uint32_t show) {
   DENTER(TOP_LAYER);
   lCondition *where = nullptr;

   lListElem *user;
   for_each_rw (user, user_name_list) {
      lCondition *new_where = nullptr;
      if (const char *user_name = lGetString(user, ST_name); is_pattern(user_name)) {
         new_where = lWhere("%T(%I p= %s)", JB_Type, JB_owner, user_name);
      } else {
         new_where = lWhere("%T(%I == %s)", JB_Type, JB_owner, user_name);
      }
      if (where == nullptr) {
         where = new_where;
      } else {
         where = lOrWhere(where, new_where);
      }
   }

   if (!(show & QSTAT_DISPLAY_PENDING)) {
      lCondition *new_where = lWhere("%T(%I->%T(!(%I m= %u)))", JB_Type, JB_ja_tasks, JAT_Type, JAT_state, JQUEUED);
      if (where == nullptr) {
         where = new_where;
      } else {
         where = lAndWhere(where, new_where);
      }
   }

   DRETURN(where);
}

lEnumeration *ocs::QHostModelBase::get_job_what() {
   //                    1           5              10             15             20             25
   return  lWhat("%T(%I %I %I %I %I %I %I %I %I %I %I %I %I %I %I %I %I %I %I %I %I %I %I %I %I %I)",
                         JB_Type,
                         JB_job_number, JB_script_file, JB_owner, JB_group, JB_type,
                         JB_pe, JB_checkpoint_name, JB_jid_predecessor_list, JB_env_list, JB_priority,
                         JB_jobshare, JB_job_name, JB_project, JB_department, JB_submission_time,
                         JB_deadline, JB_override_tickets, JB_pe_range, JB_request_set_list, JB_ja_structure,
                         JB_ja_tasks, JB_ja_n_h_ids, JB_ja_u_h_ids, JB_ja_s_h_ids, JB_ja_o_h_ids,
                         JB_ja_a_h_ids);
}

lEnumeration *ocs::QHostModelBase::get_centry_what() {
   return lWhat("%T(ALL)", CE_Type);
}

lEnumeration *ocs::QHostModelBase::get_pe_what() {
   return lWhat("%T(ALL)", PE_Type);
}

lEnumeration *ocs::QHostModelBase::get_user_set_what() {
   return lWhat("%T(ALL)", US_Type);
}


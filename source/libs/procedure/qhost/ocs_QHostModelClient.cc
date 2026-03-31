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

#include "uti/ocs_Pattern.h"
#include "uti/sge_bootstrap_files.h"
#include "uti/sge_hostname.h"
#include "uti/sge_rmon_macros.h"
#include "uti/sge.h"

#include "cull/cull.h"

#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_centry.h"
#include "sgeobj/sge_conf.h"
#include "sgeobj/sge_host.h"
#include "sgeobj/sge_ja_task.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_pe.h"
#include "sgeobj/sge_qinstance.h"
#include "sgeobj/sge_str.h"
#include "sgeobj/sge_userset.h"

#include "gdi/ocs_gdi_Client.h"
#include "gdi/ocs_gdi_Request.h"

#include "sched/sge_select_queue.h"

#include "ocs_client_print.h"

#include "qhost/ocs_QHostModelClient.h"

bool
ocs::QHostModelClient::fetch_data(lList **answer_list, const lList *hostname_list, const lList *user_name_list, uint32_t show) {
   DENTER(TOP_LAYER);

   gdi::Request gdi_multi{};

   // @todo Should be combined with the other GDI requests
   if (!gdi::Client::sge_gdi_get_permission(answer_list, &is_manager_, nullptr, nullptr, nullptr)) {
      DRETURN(false);
   }

   // execution hosts
   int eh_id;
   {
      // filter hosts by name
      lCondition *where= nullptr;
      lCondition *new_where = nullptr;
      lListElem *host;
      for_each_rw(host, hostname_list) {
         new_where = lWhere("%T(%I h= %s)", EH_Type, EH_name, lGetString(host, ST_name));
         if (!where) {
            where = new_where;
         } else {
            where = lOrWhere(where, new_where);
         }
      }

      // without host request from the user we include the global host, too.
      if (where != nullptr) {
         new_where = lWhere("%T(%I == %s)", EH_Type, EH_name, SGE_GLOBAL_NAME);
         where = lOrWhere(where, new_where);
      }

      // But we do not want the template host
      new_where = lWhere("%T(%I != %s)", EH_Type, EH_name, SGE_TEMPLATE_NAME);
      if (where)
         where = lAndWhere(where, new_where);
      else
         where = new_where;

      // request all field for hosts matching the where condition
      lEnumeration *what = lWhat("%T(ALL)", EH_Type);

      eh_id = gdi_multi.request(answer_list, gdi::Mode::RECORD, gdi::Target::EH_LIST,
                                gdi::Command::GET, gdi::SubCommand::NONE,
                                nullptr, where, what, true);
      lFreeWhat(&what);
      lFreeWhere(&where);

      if (answer_list_has_error(answer_list)) {
         DRETURN(false);
      }
   }

   // Queues
   int q_id = 0;
   if (show & QHOST_DISPLAY_JOBS || show & QHOST_DISPLAY_QUEUES) {
      lEnumeration *what = lWhat("%T(ALL)", QU_Type);

      q_id = gdi_multi.request(answer_list, gdi::Mode::RECORD, gdi::Target::CQ_LIST,
                               gdi::Command::GET, gdi::SubCommand::NONE,
                               nullptr, nullptr, what, true);
      lFreeWhat(&what);

      if (answer_list_has_error(answer_list)) {
         DRETURN(false);
      }
   }

   // Jobs
   int j_id = 0;
   if ((show & QHOST_DISPLAY_JOBS) == QHOST_DISPLAY_JOBS) {

      lCondition *new_where = nullptr;
      lCondition *where = nullptr;
      lListElem *user;
      for_each_rw (user, user_name_list) {
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
         new_where = lWhere("%T(%I->%T(!(%I m= %u)))", JB_Type, JB_ja_tasks, JAT_Type, JAT_state, JQUEUED);
         if (where == nullptr) {
            where = new_where;
         } else {
            where = lAndWhere(where, new_where);
         }
      }

      //                                 1           5              10             15             20             25
      lEnumeration *what = lWhat("%T(%I %I %I %I %I %I %I %I %I %I %I %I %I %I %I %I %I %I %I %I %I %I %I %I %I %I)",
                                 JB_Type,
                                 JB_job_number, JB_script_file, JB_owner, JB_group, JB_type,
                                 JB_pe, JB_checkpoint_name, JB_jid_predecessor_list, JB_env_list, JB_priority,
                                 JB_jobshare, JB_job_name, JB_project, JB_department, JB_submission_time,
                                 JB_deadline, JB_override_tickets, JB_pe_range, JB_request_set_list, JB_ja_structure,
                                 JB_ja_tasks, JB_ja_n_h_ids, JB_ja_u_h_ids, JB_ja_s_h_ids, JB_ja_o_h_ids,
                                 JB_ja_a_h_ids);

      j_id = gdi_multi.request(answer_list, gdi::Mode::RECORD, gdi::Target::JB_LIST, gdi::Command::GET,
                               gdi::SubCommand::NONE, nullptr, where, what, true);
      lFreeWhat(&what);
      lFreeWhere(&where);

      if (answer_list_has_error(answer_list)) {
         DRETURN(false);
      }
   }

   // Complexes
   int ce_id;
   {
      lEnumeration *what = lWhat("%T(ALL)", CE_Type);
      ce_id = gdi_multi.request(answer_list, gdi::Mode::RECORD, gdi::Target::CE_LIST, gdi::Command::GET,
                                gdi::SubCommand::NONE, nullptr, nullptr, what, true);
      lFreeWhat(&what);

      if (answer_list_has_error(answer_list)) {
         DRETURN(false);
      }
   }

   // Parallel environments
   int pe_id;
   {
      lEnumeration *what = lWhat("%T(ALL)", PE_Type);
      pe_id = gdi_multi.request(answer_list, gdi::Mode::RECORD, gdi::Target::PE_LIST, gdi::Command::GET,
                                gdi::SubCommand::NONE, nullptr, nullptr, what, true);
      lFreeWhat(&what);
      if (answer_list_has_error(answer_list)) {
         DRETURN(false);
      }
   }

   /*
   ** user list
   */
   int acl_id;
   {
      lEnumeration *what = lWhat("%T(ALL)", US_Type);
      acl_id = gdi_multi.request(answer_list, gdi::Mode::RECORD, gdi::Target::US_LIST, gdi::Command::GET,
                                 gdi::SubCommand::NONE, nullptr, nullptr, what, true);
      lFreeWhat(&what);

      if (answer_list_has_error(answer_list)) {
         DRETURN(false);
      }
   }

   // Configuration
   int gc_id;
   {
      lCondition *where = lWhere("%T(%I c= %s)", CONF_Type, CONF_name, SGE_GLOBAL_NAME);
      lEnumeration *what = lWhat("%T(ALL)", CONF_Type);

      gc_id = gdi_multi.request(answer_list, gdi::Mode::SEND, gdi::Target::CONF_LIST, gdi::Command::GET,
                                gdi::SubCommand::NONE, nullptr, where, what, true);
      gdi_multi.wait();
      lFreeWhat(&what);
      lFreeWhere(&where);

      if (answer_list_has_error(answer_list)) {
         DRETURN(false);
      }
   }

   // Delete the ok message
   lFreeList(answer_list);

   /*
   ** handle results
   */

   // Execution hosts
   gdi_multi.get_response(answer_list, gdi::Command::GET, gdi::SubCommand::NONE, gdi::Target::EH_LIST, eh_id, &exechost_list_);
   if (answer_list_has_error(answer_list)) {
      answer_list_output(answer_list);
      DRETURN(false);
   }

   // Cluster queues
   if (show & QHOST_DISPLAY_JOBS || show & QHOST_DISPLAY_QUEUES) {
      gdi_multi.get_response(answer_list, gdi::Command::GET, gdi::SubCommand::NONE, gdi::Target::CQ_LIST, q_id, &queue_list_);
      if (answer_list_has_error(answer_list)) {
         answer_list_output(answer_list);
         DRETURN(false);
      }
   }

   // Jobs
   if ((show & QHOST_DISPLAY_JOBS) == QHOST_DISPLAY_JOBS) {
      gdi_multi.get_response(answer_list, gdi::Command::GET, gdi::SubCommand::NONE, gdi::Target::JB_LIST, j_id, &job_list_);
      if (answer_list_has_error(answer_list)) {
         answer_list_output(answer_list);
         DRETURN(false);
      }
   }

   // complexes
   gdi_multi.get_response(answer_list, gdi::Command::GET, gdi::SubCommand::NONE, gdi::Target::CE_LIST, ce_id, &centry_list_);
   if (answer_list_has_error(answer_list)) {
      answer_list_output(answer_list);
      DRETURN(false);
   }

   // PE's
   gdi_multi.get_response(answer_list, gdi::Command::GET, gdi::SubCommand::NONE, gdi::Target::PE_LIST, pe_id, &pe_list_);
   if (answer_list_has_error(answer_list)) {
      answer_list_output(answer_list);
      DRETURN(false);
   }

   // ACL's
   gdi_multi.get_response(answer_list, gdi::Command::GET, gdi::SubCommand::NONE, gdi::Target::US_LIST, acl_id, &acl_list_);
   if (answer_list_has_error(answer_list)) {
      answer_list_output(answer_list);
      DRETURN(false);
   }

   // get configuration
   gdi_multi.get_response(answer_list, gdi::Command::GET, gdi::SubCommand::NONE, gdi::Target::CONF_LIST, gc_id, &config_list_);
   if (answer_list_has_error(answer_list)) {
      answer_list_output(answer_list);
      DRETURN(false);
   }

   DRETURN(true);
}

bool
ocs::QHostModelClient::prepare_data(lList **answer_list, const lList *resource_match_list, const uint32_t show) const {
   DENTER(TOP_LAYER);
   // first step: prepare configuration
   if (lFirst(config_list_)) {
      const char *cell_root = bootstrap_get_cell_root();
      const uint32_t progid = component_get_component_id();

      merge_configuration(nullptr, progid, cell_root, lFirstRW(config_list_), nullptr, nullptr);
   }

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

void
ocs::QHostModelClient::filter_data(const lList *resource_match_list) {
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

void
ocs::QHostModelClient::sort_data() {
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
ocs::QHostModelClient::make_snapshot(lList **answer_list, QHostParameter &parameter) {
   const lList *hostname_list = parameter.get_hostname_list();
   const lList *user_name_list = parameter.get_user_name_list();
   const lList *resource_match_list = parameter.get_resource_match_list();
   const uint32_t show = parameter.get_show();

   // fetch data from qmaster
   if (!fetch_data(answer_list, hostname_list, user_name_list, show)) {
      return false;
   }

   // init raw data received from qmaster
   if (!prepare_data(answer_list, resource_match_list, show)) {
      return false;
   }

   // filter and sort data for output
   filter_data(resource_match_list);
   sort_data();
   return true;
}

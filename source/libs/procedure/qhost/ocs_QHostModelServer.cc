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

#include "uti/sge_rmon_macros.h"

#include "sgeobj/ocs_DataStore.h"
#include "sgeobj/sge_manop.h"

#include "ocs_QHostModelBase.h"
#include "ocs_QHostModelServer.h"

#include "ocs_client_print.h"
#include "ocs_QHostModelClient.h"


bool ocs::QHostModelServer::fetch_data(lList **answer_list, const lList *hostname_list, const lList *user_name_list, uint32_t show) {
   DENTER(TOP_LAYER);


   // Has the request user the manager role
   const lList *master_manager_list = *DataStore::get_master_list(SGE_TYPE_MANAGER);
   is_manager_ = manop_is_manager(packet, master_manager_list);

   // Fetch either all host or only the selected ones
   const lList *master_host_list = *DataStore::get_master_list(SGE_TYPE_EXECHOST);
   exec_host_list_ = lSelect("", master_host_list, get_host_where(hostname_list), get_host_what());

   if (show & QHOST_DISPLAY_JOBS || show & QHOST_DISPLAY_QUEUES) {
      const lList *master_cqueue_list = *DataStore::get_master_list(SGE_TYPE_CQUEUE);
      queue_list_ = lSelect("", master_cqueue_list, nullptr, get_queue_what());

   }

   if ((show & QHOST_DISPLAY_JOBS) == QHOST_DISPLAY_JOBS) {
      const lList *master_job_list = *DataStore::get_master_list(SGE_TYPE_JOB);
      job_list_ = lSelect("", master_job_list, get_job_where(user_name_list, show), get_job_what());
   }

   const lList *master_centry_list = *DataStore::get_master_list(SGE_TYPE_CENTRY);
   centry_list_ = lSelect("", master_centry_list, nullptr, get_centry_what());

   const lList *master_pe_list = *DataStore::get_master_list(SGE_TYPE_PE);
   pe_list_ = lSelect("", master_pe_list, nullptr, get_pe_what());

   // QHostModelClient fetches the configuration additionally which is not required in the server context
   ;

   DRETURN(true);
}

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

#include "cull/cull_list.h"

#include "uti/sge_rmon_macros.h"

#include "sgeobj/sge_advance_reservation.h"
#include "sgeobj/sge_ulong.h"
#include "sgeobj/sge_str.h"

#include "gdi/ocs_gdi_Client.h"

#include "qrstat/ocs_QRStatModelBase.h"
#include "qrstat/ocs_QRStatParameterClient.h"

ocs::QRStatModelBase::~QRStatModelBase() {
   DENTER(TOP_LAYER);
   lFreeList(&ar_list_);
   DRETURN_VOID;
}

lEnumeration *ocs::QRStatModelBase::get_ar_what(QRStatParameter& parameter) {
   // Core attributes
   constexpr int core_nm[] = {
      AR_id,
      AR_name,
      AR_owner,
      AR_start_time,
      AR_end_time,
      AR_duration,
      AR_state,
      AR_reserved_queues,
      AR_reserved_hosts,
      NoName
   };
   lEnumeration *what = lIntVector2What(AR_Type, core_nm);

   // for XML or JSON output
   if (parameter.get_output_format() != QRStatParameterClient::OutputFormat::PLAIN) {
      constexpr int xml_nm[] = {
         AR_account,
         AR_owner,
         AR_group,
         AR_submission_time,
         AR_verify,
         AR_error_handling,
         AR_checkpoint_name,
         AR_resource_list,
         AR_resource_utilization,
         AR_queue_list,
         AR_granted_slots,
         AR_mail_options,
         AR_mail_list,
         AR_pe,
         AR_pe_range,
         AR_acl_list,
         AR_xacl_list,
         AR_type,
         NoName
      };
      lEnumeration *more_what = lIntVector2What(AR_Type, xml_nm);
      lMergeWhat(&what, &more_what);
   }

   // required for full AR output
   if (!parameter.is_summary()) {
      constexpr int full_nm[] = {
         AR_account,
         AR_owner,
         AR_group,
         AR_submission_time,
         AR_verify,
         AR_error_handling,
         AR_checkpoint_name,
         AR_resource_list,
         AR_resource_utilization,
         AR_queue_list,
         AR_granted_slots,
         AR_mail_options,
         AR_mail_list,
         AR_pe,
         AR_pe_range,
         AR_master_queue_list,
         AR_acl_list,
         AR_xacl_list,
         AR_type,
         AR_reserved_queues,
         AR_reserved_hosts,
         AR_binding,
         AR_granted_resources_list,
         NoName
      };
      lEnumeration *more_what = lIntVector2What(AR_Type, full_nm);
      lMergeWhat(&what, &more_what);
   }

   // required for -explain
   if (parameter.is_explain()) {
      constexpr int explain_nm[] = {
         AR_error_handling,
         NoName
      };

      lEnumeration *more_what = lIntVector2What(AR_Type, explain_nm);
      lMergeWhat(&what, &more_what);
   }
   return what;
}

lCondition *ocs::QRStatModelBase::get_ar_where(QRStatParameter& parameter) {
   DENTER(TOP_LAYER);
   lCondition *where_AR_Type = nullptr;

   // Summay or full view
   if (!parameter.is_summary()) {
      lCondition *where = nullptr;

      for_each_ep_lv(elem, parameter.get_ar_id_list()) {
         lCondition *tmp_where = nullptr;
         uint32_t value = lGetUlong(elem, ULNG_value);

         tmp_where = lWhere("%T(%I == %u)", AR_Type, AR_id, value);
         if (tmp_where != nullptr) {
            if (where == nullptr) {
               where = tmp_where;
            } else {
               where = lOrWhere(where, tmp_where);
            }
         }
      }
      where_AR_Type = where;
   } else {
      // filter users
      parameter.transform_user_list();

      lCondition *where = nullptr;
      for_each_ep_lv(elem, parameter.get_user_list()) {
         const char *name = lGetString(elem, ST_name);

         lCondition *tmp_where = lWhere("%T(%I p= %s)", AR_Type, AR_owner, name);
         if (tmp_where != nullptr) {
            if (where == nullptr) {
               where = tmp_where;
            } else {
               where = lOrWhere(where, tmp_where);
            }
         }
      }
      if (where_AR_Type == nullptr) {
         where_AR_Type = where;
      } else {
         where_AR_Type = lAndWhere(where_AR_Type, where);
      }
   }
   return where_AR_Type;
}

bool ocs::QRStatModelBase::fetch_data(lList **answer_list, QRStatParameter& parameter) {
   DENTER(TOP_LAYER);
   // has to be overwritten by child classes
   DRETURN(true);
}

bool ocs::QRStatModelBase::make_snapshot(lList **answer_list, QRStatParameter &parameter) {
   DENTER(TOP_LAYER);

   if (!fetch_data(answer_list, parameter)) {
      DRETURN(false);
   }

   DRETURN(true);
}

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

#include "uti/sge_rmon_macros.h"
#include "uti/sge_hostname.h"

#include "sgeobj/cull/sge_param_SPP_L.h"
#include "sgeobj/sge_str.h"
#include "sgeobj/sge_ulong.h"
#include "sgeobj/sge_var.h"

#include "qhost/ocs_QHostParameter.h"

#include "ocs_client_print.h"
#include "ocs_ProcedureParameter.h"

extern char **environ;

void ocs::QHostParameter::free_data() {
   // @todo free other lists too
   lFreeList(&hostname_list_);
   lFreeList(&user_name_list_);
   lFreeList(&resource_match_list_);
}

lList *ocs::QHostParameter::get_bundle(const std::string& procedure_name) {
   DENTER(TOP_LAYER);
   lListElem *ep = nullptr;

   // Get the name-value-list that was initialized by the base class with the procedure name
   lList *bundle =  ProcedureParameter::get_bundle(procedure_name);
   //lList *name_value_list = lGetListRW(lGetElemStrRW(bundle, SPP_name, NAME_VALUE_LIST), SPP_value_list);

   // -h
   ep = lAddElemStr(&bundle, SPP_name, HOSTNAME_LIST, SPP_Type);
   lSetList(ep, SPP_value_list, hostname_list_);

   // -u
   ep = lAddElemStr(&bundle, SPP_name, USER_NAME_LIST, SPP_Type);
   lSetList(ep, SPP_value_list, user_name_list_);

   // -l
   ep = lAddElemStr(&bundle, SPP_name, RESOURCE_LIST, SPP_Type);
   lSetList(ep, SPP_value_list, resource_match_list_);

   // -F
   ep = lAddElemStr(&bundle, SPP_name, RESOURCE_VISIBLE_LIST, SPP_Type);
   lSetList(ep, SPP_value_list, resource_visible_list_);

   // flags for used switches: -F -q -j -u
   lList *show_list = nullptr;
   lAddElemUlong(&show_list, ULNG_value, show_, ULNG_Type);
   ep = lAddElemStr(&bundle, SPP_name, SHOW, SPP_Type);
   lSetList(ep, SPP_value_list, show_list);

   // -fmt
   lList *output_format_list = nullptr;
   lAddElemUlong(&output_format_list, ULNG_value, static_cast<uint32_t>(output_format_), ULNG_Type);
   ep = lAddElemStr(&bundle, SPP_name, OUTPUT_FORMAT, SPP_Type);
   lSetList(ep, SPP_value_list, output_format_list);


   DRETURN(bundle);
}

void ocs::QHostParameter::set_bundle(lList **bundle) {
   DENTER(TOP_LAYER);

   // procedure name ...
   const lListElem *name_value_param = lGetElemStr(*bundle, SPP_name, NAME_VALUE_LIST);
   const lList *name_value_list = lGetList(name_value_param, SPP_value_list);
   procedure_name_ = lGetString(lFirst(name_value_list), VA_variable);

   DPRINTF("procedure name: %s\n", procedure_name_.c_str());

   // -fmt
   const lListElem *output_format_param = lGetElemStr(*bundle, SPP_name, OUTPUT_FORMAT);
   const lList *output_format_list = lGetList(output_format_param, SPP_value_list);
   output_format_ = static_cast<OutputFormat>(lGetUlong(lFirst(output_format_list), ULNG_value));

   DPRINTF("output format: %d\n", output_format_);

   // flags for used switches: -F -q -j -u
   const lListElem *show_param = lGetElemStr(*bundle, SPP_name, SHOW);
   const lList *show_list = lGetList(show_param, SPP_value_list);
   show_ = lGetUlong(lFirst(show_list), ULNG_value);

   DPRINTF("show: %d\n", show_);

   // -F
   lListElem *resource_visible_param = lGetElemStrRW(*bundle, SPP_name, RESOURCE_VISIBLE_LIST);
   resource_visible_list_ = nullptr;
   lXchgList(resource_visible_param, SPP_value_list, &resource_visible_list_);

   DPRINTF("resource_visible: %p\n", resource_visible_list_);

   // -l
   lListElem *resource_match_param = lGetElemStrRW(*bundle, SPP_name, RESOURCE_LIST);
   resource_match_list_ = nullptr;
   lXchgList(resource_match_param, SPP_value_list, &resource_match_list_);

   DPRINTF("resource_match: %p\n", resource_match_list_);

   // -u
   lListElem *user_name_param = lGetElemStrRW(*bundle, SPP_name, USER_NAME_LIST);
   user_name_list_ = nullptr;
   lXchgList(user_name_param, SPP_value_list, &user_name_list_);

   DPRINTF("user_name: %p\n", user_name_list_);

   // -h
   lListElem *host_name_param = lGetElemStrRW(*bundle, SPP_name, HOSTNAME_LIST);
   hostname_list_ = nullptr;
   lXchgList(host_name_param, SPP_value_list, &hostname_list_);

   DPRINTF("hostname: %p\n", hostname_list_);

   // free the bundle
   lFreeList(bundle);

   DRETURN_VOID;
}

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

#include "sgeobj/cull/sge_param_SPP_L.h"
#include "sgeobj/sge_var.h"

#include "ocs_ProcedureParameter.h"

#include "uti/sge_log.h"


void ocs::ProcedureParameter::add_parameter_bundle(lList *bundle, const std::string& param_name, lList *parameter) {
   SGE_ASSERT(bundle != nullptr);
   lListElem *bundle_elem = lAddElemStr(&bundle, SPP_name, param_name.c_str(), SPP_Type);
   lSetList(bundle_elem, SPP_value_list, parameter);
}

lList *
ocs::ProcedureParameter::get_bundle() {
   lList *bundle = nullptr;

   // procedure name, (client env variables or other things ...)
   lList *name_value_list = nullptr;
   lListElem *ep = lAddElemStr(&name_value_list, VA_variable, PROCEDURE, VA_Type);
   lSetString(ep, VA_value, procedure_name_.c_str());
   ep = lAddElemStr(&bundle, SPP_name, NAME_VALUE_LIST, SPP_Type);
   lSetList(ep, SPP_value_list, name_value_list);
   return bundle;
}

std::string ocs::ProcedureParameter::get_procedure_from_bundle(const lList *parameter_bundle) {
   const lListElem *name_value_param = lGetElemStr(parameter_bundle, SPP_name, NAME_VALUE_LIST);
   const lList *name_value_list = lGetList(name_value_param, SPP_value_list);
   const lListElem *procedure_elem = lGetElemStr(name_value_list, VA_variable, PROCEDURE);
   return lGetString(procedure_elem, VA_value);
}

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

#include <sstream>
#include <memory>

#include "uti/sge_rmon_macros.h"
#include "uti/sge_log.h"

#include "sgeobj/sge_answer.h"

#include "procedure/ocs_ProcedureParameter.h"
#include "procedure/qhost/ocs_QHostModelServer.h"
#include "procedure/qhost/ocs_QHostContoller.h"
#include "procedure/qhost/ocs_QHostViewPlain.h"
#include "procedure/qhost/ocs_QHostViewXML.h"
#include "procedure/qhost/ocs_QHostViewJSON.h"

#include "sge_c_gdi_procedure.h"

#include "qquota/ocs_QQuotaController.h"
#include "qquota/ocs_QQuotaModelServer.h"
#include "qquota/ocs_QQuotaParameter.h"
#include "qquota/ocs_QQuotaViewBase.h"
#include "qquota/ocs_QQuotaViewJSON.h"
#include "qquota/ocs_QQuotaViewPlain.h"
#include "qquota/ocs_QQuotaViewXML.h"

void sge_c_gdi_procedure_qhost_exec(ocs::gdi::Packet *packet, ocs::gdi::Task *task, std::ostringstream &os) {
   DENTER(TOP_LAYER);

   // Create parameter object for the stored procedure
   ocs::QHostParameter parameter(&task->data_list);

   // Create the server side model and make a snapshot of the required data
   ocs::QHostModelServer model(packet, task);
   if (!model.make_snapshot(&task->answer_list, parameter)) {
      // error was written by make_snapshot()
      DRETURN_VOID;
   }

   // prepare view to show output
   std::unique_ptr<ocs::QHostViewBase> view;
   switch (parameter.get_output_format()) {
      case ocs::ProcedureParameter::OutputFormat::XML:
         view = std::make_unique<ocs::QHostViewXML>(parameter);
         break;
      case ocs::ProcedureParameter::OutputFormat::PLAIN:
         view = std::make_unique<ocs::QHostViewPlain>(parameter);
         break;
      case ocs::ProcedureParameter::OutputFormat::JSON:
         view = std::make_unique<ocs::QHostViewJSON>(parameter);
         break;
   }

   // Should not be possible unless client sent a wrong object with unexpected values for expected parameter
   if (view == nullptr) {
      snprintf(SGE_EVENT, SGE_EVENT_SIZE, "creating view failed in " SFQ, __func__);
      answer_list_add(&(task->answer_list), SGE_EVENT, STATUS_ENOIMP, ANSWER_QUALITY_ERROR);
      DRETURN_VOID;
   }

   // process request and show output
   ocs::QHostController controller(os);
   controller.process_request(parameter, model, *view);

   DRETURN_VOID;
}

void sge_c_gdi_procedure_qhost_prepare_response(ocs::gdi::Packet *packet, ocs::gdi::Task *task, std::ostringstream &os) {
   DENTER(TOP_LAYER);
   // Prepare a response
   ocs::ProcedureParameter response(prognames[QHOST]);
   lList *bundle = response.get_bundle();

   // Add the output to the bundle
   lList *output_list = nullptr;
   lAddElemStr(&output_list, ST_name, os.str().c_str(), ST_Type);
   ocs::ProcedureParameter::add_parameter_bundle(bundle, ocs::ProcedureParameter::RESPONSE, output_list);

   // Pass responsibility for the bundle to gdi
   task->data_list = bundle;
   DRETURN_VOID;
}

void sge_c_gdi_procedure_qquota_exec(ocs::gdi::Packet *packet, ocs::gdi::Task *task, std::ostringstream &os) {
   DENTER(TOP_LAYER);

   // Create parameter object for the stored procedure
   ocs::QQuotaParameter parameter(&task->data_list);

   // Create the server side model and make a snapshot of the required data
   ocs::QQuotaModelServer model(packet, task);
   if (!model.make_snapshot(&task->answer_list, parameter)) {
      // error was written by make_snapshot()
      DRETURN_VOID;
   }

   // prepare view to show output
   std::unique_ptr<ocs::QQuotaViewBase> view;
   switch (parameter.get_output_format()) {
      case ocs::ProcedureParameter::OutputFormat::XML:
         view = std::make_unique<ocs::QQuotaViewXML>(parameter);
         break;
      case ocs::ProcedureParameter::OutputFormat::PLAIN:
         view = std::make_unique<ocs::QQuotaViewPlain>(parameter);
         break;
      case ocs::ProcedureParameter::OutputFormat::JSON:
         view = std::make_unique<ocs::QQuotaViewJSON>(parameter);
         break;
   }

   // Should not be possible unless client sent a wrong object with unexpected values for expected parameter
   if (view == nullptr) {
      snprintf(SGE_EVENT, SGE_EVENT_SIZE, "creating view failed in " SFQ, __func__);
      answer_list_add(&(task->answer_list), SGE_EVENT, STATUS_ENOIMP, ANSWER_QUALITY_ERROR);
      DRETURN_VOID;
   }

   // process request and show output
   ocs::QQuotaController controller(os);
   controller.process_request(parameter, model, *view);

   DRETURN_VOID;
}

void sge_c_gdi_procedure_qquota_prepare_response(ocs::gdi::Packet *packet, ocs::gdi::Task *task, std::ostringstream &os) {
   DENTER(TOP_LAYER);
   // Prepare a response
   ocs::ProcedureParameter response(prognames[QHOST]);
   lList *bundle = response.get_bundle();

   // Add the output to the bundle
   lList *output_list = nullptr;
   lAddElemStr(&output_list, ST_name, os.str().c_str(), ST_Type);
   ocs::ProcedureParameter::add_parameter_bundle(bundle, ocs::ProcedureParameter::RESPONSE, output_list);

   // Pass responsibility for the bundle to gdi
   task->data_list = bundle;
   DRETURN_VOID;
}


void sge_c_gdi_procedure(gdi_object_t *ao, ocs::gdi::Packet *packet, ocs::gdi::Task *task, ocs::gdi::Command cmd,
                         ocs::gdi::SubCommand sub_command, monitoring_t *monitor) {
   DENTER(TOP_LAYER);

   // get the name of the procedure that should be called
   const std::string procedure_name = ocs::ProcedureParameter::get_procedure_from_bundle(task->data_list);

   // create an instance of the correct parameter class
   if (procedure_name == prognames[QHOST]) {
      std::ostringstream out_ss;
      sge_c_gdi_procedure_qhost_exec(packet, task, out_ss);
      sge_c_gdi_procedure_qhost_prepare_response(packet, task, out_ss);
   } else if (procedure_name == prognames[QQUOTA]) {
      std::ostringstream out_ss;
      sge_c_gdi_procedure_qquota_exec(packet, task, out_ss);
      sge_c_gdi_procedure_qquota_prepare_response(packet, task, out_ss);
   } else {
      snprintf(SGE_EVENT, SGE_EVENT_SIZE, "requested stored procedure " SFQ " is not available",
               procedure_name.c_str());
      answer_list_add(&(task->answer_list), SGE_EVENT, STATUS_ENOIMP, ANSWER_QUALITY_ERROR);
   }

   DRETURN_VOID;
}

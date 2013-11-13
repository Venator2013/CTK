/*=============================================================================

  Plugin: org.commontk.xnat

  Copyright (c) University College London,
    Centre for Medical Image Computing

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

=============================================================================*/

#ifndef CTKXNATDATAMODEL_H
#define CTKXNATDATAMODEL_H

#include "ctkXNATCoreExport.h"

#include "ctkXnatObject.h"

class ctkXnatDataModelPrivate;
class ctkXnatProject;
class ctkXnatSession;

class ctkXnatDataModel : public ctkXnatObject
{

public:

  CTK_XNAT_OBJECT(ctkXnatDataModel, ctkXnatObject, "https://central.xnat.org/schemas/xnat/xnat.xsd")

  ctkXnatDataModel(ctkXnatSession* connection);

  QList<ctkXnatProject*> projects() const;

  ctkXnatSession* session() const;

private:

  QString resourceUri() const;

  virtual void fetchImpl();

  Q_DECLARE_PRIVATE(ctkXnatDataModel)
};

#endif
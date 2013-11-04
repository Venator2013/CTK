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

#ifndef ctkXnatFile_h
#define ctkXnatFile_h

#include "ctkXNATCoreExport.h"

#include "ctkXnatObject.h"

class ctkXnatConnection;
class ctkXnatFilePrivate;

class CTK_XNAT_CORE_EXPORT ctkXnatFile : public ctkXnatObject
{

public:

  typedef QSharedPointer<ctkXnatFile> Pointer;
  typedef QWeakPointer<ctkXnatFile> WeakPointer;
  
  static Pointer Create();
  virtual ~ctkXnatFile();
  
//  const QString& uri() const;
//  void setUri(const QString& uri);

  void download(const QString& filename);
  void upload(const QString& filename);

  void reset();
  
private:
  
  friend class qRestResult;
  explicit ctkXnatFile();
  virtual void fetchImpl();
  
  Q_DECLARE_PRIVATE(ctkXnatFile);
  Q_DISABLE_COPY(ctkXnatFile);
};

#endif
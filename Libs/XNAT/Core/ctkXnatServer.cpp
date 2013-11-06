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

#include "ctkXnatServer.h"

#include "ctkXnatObjectPrivate.h"
#include "ctkXnatConnection.h"

class ctkXnatServerPrivate : public ctkXnatObjectPrivate
{
  explicit ctkXnatServerPrivate(ctkXnatConnection* connection);
  virtual ~ctkXnatServerPrivate();

private:
  friend class ctkXnatServer;

  ctkXnatConnection* connection;
};

ctkXnatServerPrivate::ctkXnatServerPrivate(ctkXnatConnection* connection)
: ctkXnatObjectPrivate()
, connection(connection)
{
}

ctkXnatServerPrivate::~ctkXnatServerPrivate()
{
}

ctkXnatServer::ctkXnatServer(ctkXnatConnection* connection)
: ctkXnatObject(*new ctkXnatServerPrivate(connection))
{
}

void ctkXnatServer::fetchImpl()
{
  this->connection()->fetch(this);
}

ctkXnatConnection* ctkXnatServer::connection() const
{
  Q_D(const ctkXnatServer);
  return d->connection;
}
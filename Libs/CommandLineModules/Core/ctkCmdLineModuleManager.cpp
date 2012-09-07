/*=============================================================================
  
  Library: CTK
  
  Copyright (c) German Cancer Research Center,
    Division of Medical and Biological Informatics
    
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

#include "ctkCmdLineModuleManager.h"

#include "ctkCmdLineModuleBackend.h"
#include "ctkCmdLineModuleFrontend.h"
#include "ctkCmdLineModuleCache_p.h"
#include "ctkCmdLineModuleFuture.h"
#include "ctkCmdLineModuleXmlValidator.h"
#include "ctkCmdLineModuleReference.h"
#include "ctkCmdLineModuleReference_p.h"

#include <ctkException.h>

#include <QFileInfo>
#include <QDir>
#include <QStringList>
#include <QBuffer>
#include <QUrl>
#include <QHash>
#include <QList>
#include <QMutex>

#include <QFuture>

//----------------------------------------------------------------------------
struct ctkCmdLineModuleManagerPrivate
{
  ctkCmdLineModuleManagerPrivate(ctkCmdLineModuleManager::ValidationMode mode, const QString& cacheDir)
    : ValidationMode(mode)
  {
    QFileInfo fileInfo(cacheDir);
    if (!fileInfo.exists())
    {
      if (!QDir().mkpath(cacheDir))
      {
        qWarning() << "Command line module cache disabled. Directory" << cacheDir << "could not be created.";
        return;
      }
    }

    if (fileInfo.isWritable())
    {
      ModuleCache.reset(new ctkCmdLineModuleCache(cacheDir));
    }
    else
    {
      qWarning() << "Command line module cache disabled. Directory" << cacheDir << "is not writable.";
    }
  }

  void checkBackends_unlocked(const QUrl& location)
  {
    if (!this->SchemeToBackend.contains(location.scheme()))
    {
      throw ctkInvalidArgumentException(QString("No suitable backend registered for module at ") + location.toString());
    }
  }

  QMutex Mutex;
  QHash<QString, ctkCmdLineModuleBackend*> SchemeToBackend;
  QHash<QUrl, ctkCmdLineModuleReference> LocationToRef;
  QScopedPointer<ctkCmdLineModuleCache> ModuleCache;

  const ctkCmdLineModuleManager::ValidationMode ValidationMode;
};

//----------------------------------------------------------------------------
ctkCmdLineModuleManager::ctkCmdLineModuleManager(ValidationMode validationMode, const QString& cacheDir)
  : d(new ctkCmdLineModuleManagerPrivate(validationMode, cacheDir))
{
}

//----------------------------------------------------------------------------
ctkCmdLineModuleManager::~ctkCmdLineModuleManager()
{
}

//----------------------------------------------------------------------------
void ctkCmdLineModuleManager::registerBackend(ctkCmdLineModuleBackend *backend)
{
  QMutexLocker lock(&d->Mutex);

  QList<QString> supportedSchemes = backend->schemes();

  // Check if there is already a backend registerd for any of the
  // supported schemes. We only supported one backend per scheme.
  foreach (QString scheme, supportedSchemes)
  {
    if (d->SchemeToBackend.contains(scheme))
    {
      throw ctkInvalidArgumentException(QString("A backend for scheme %1 is already registered.").arg(scheme));
    }
  }

  // All good
  foreach (QString scheme, supportedSchemes)
  {
    d->SchemeToBackend[scheme] = backend;
  }
}

//----------------------------------------------------------------------------
ctkCmdLineModuleReference
ctkCmdLineModuleManager::registerModule(const QUrl &location)
{
  QByteArray xml;
  ctkCmdLineModuleBackend* backend = NULL;
  {
    QMutexLocker lock(&d->Mutex);

    d->checkBackends_unlocked(location);

    // If the module is already registered, just return the reference
    if (d->LocationToRef.contains(location))
    {
      return d->LocationToRef[location];
    }

    backend = d->SchemeToBackend[location.scheme()];
  }

  bool fromCache = false;
  qint64 newTimeStamp = 0;
  if (d->ModuleCache)
  {
    newTimeStamp = backend->timeStamp(location);
    if (d->ModuleCache->timeStamp(location) < newTimeStamp)
    {
      // newly fetch the XML description
      try
      {
        xml = backend->rawXmlDescription(location);
      }
      catch (...)
      {
        // cache the failed attempt
        d->ModuleCache->cacheXmlDescription(location, newTimeStamp, QByteArray());
        throw;
      }
    }
    else
    {
      // use the cached XML description
      xml = d->ModuleCache->rawXmlDescription(location);
      fromCache = true;
    }
  }
  else
  {
    xml = backend->rawXmlDescription(location);
  }

  if (xml.isEmpty())
  {
    if (!fromCache && d->ModuleCache)
    {
      d->ModuleCache->cacheXmlDescription(location, newTimeStamp, QByteArray());
    }
    throw ctkInvalidArgumentException(QString("No XML output available from ") + location.toString());
  }

  ctkCmdLineModuleReference ref;
  ref.d->Location = location;
  ref.d->RawXmlDescription = xml;
  ref.d->Backend = backend;

  if (d->ValidationMode != SKIP_VALIDATION)
  {
    // validate the outputted xml description
    QBuffer input(&xml);
    input.open(QIODevice::ReadOnly);

    ctkCmdLineModuleXmlValidator validator(&input);
    if (!validator.validateInput())
    {
      if (d->ModuleCache)
      {
        // validation failed, cache the description anyway
        d->ModuleCache->cacheXmlDescription(location, newTimeStamp, xml);
      }

      if (d->ValidationMode == STRICT_VALIDATION)
      {
        throw ctkInvalidArgumentException(QString("Validating module at %1 failed: %2")
                                          .arg(location.toString()).arg(validator.errorString()));
      }
      else
      {
        ref.d->XmlValidationErrorString = validator.errorString();
      }
    }
    else
    {
      if (d->ModuleCache && newTimeStamp > 0)
      {
        // successfully validated the xml, cache it
        d->ModuleCache->cacheXmlDescription(location, newTimeStamp, xml);
      }
    }
  }
  else
  {
    if (!fromCache && d->ModuleCache)
    {
      // cache it
      d->ModuleCache->cacheXmlDescription(location, newTimeStamp, xml);
    }
  }

  {
    QMutexLocker lock(&d->Mutex);
    // Check that we don't have a race condition
    if (d->LocationToRef.contains(location))
    {
      // Another thread registered a module with the same location
      return d->LocationToRef[location];
    }
    d->LocationToRef[location] = ref;
  }

  emit moduleRegistered(ref);
  return ref;
}

//----------------------------------------------------------------------------
void ctkCmdLineModuleManager::unregisterModule(const ctkCmdLineModuleReference& ref)
{
  if (!ref) return;

  {
    QMutexLocker lock(&d->Mutex);
    if (!d->LocationToRef.contains(ref.location()))
    {
      return;
    }
    d->LocationToRef.remove(ref.location());
    if (d->ModuleCache)
    {
      d->ModuleCache->removeCacheEntry(ref.location());
    }
  }
  emit moduleUnregistered(ref);
}

//----------------------------------------------------------------------------
ctkCmdLineModuleReference ctkCmdLineModuleManager::moduleReference(const QUrl &location) const
{
  QMutexLocker lock(&d->Mutex);
  QHash<QUrl, ctkCmdLineModuleReference>::const_iterator iter = d->LocationToRef.find(location);
  return (iter == d->LocationToRef.end()) ? ctkCmdLineModuleReference() : iter.value();
}

//----------------------------------------------------------------------------
QList<ctkCmdLineModuleReference> ctkCmdLineModuleManager::moduleReferences() const
{
  QMutexLocker lock(&d->Mutex);
  return d->LocationToRef.values();
}

//----------------------------------------------------------------------------
ctkCmdLineModuleFuture ctkCmdLineModuleManager::run(ctkCmdLineModuleFrontend *frontend)
{
  QMutexLocker lock(&d->Mutex);
  d->checkBackends_unlocked(frontend->location());

  ctkCmdLineModuleFuture future = d->SchemeToBackend[frontend->location().scheme()]->run(frontend);
  frontend->setFuture(future);
  emit frontend->started();
  return future;
}

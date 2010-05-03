/*=========================================================================

  Library:   CTK
 
  Copyright (c) 2010  Kitware Inc.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.commontk.org/LICENSE

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
 
=========================================================================*/

#ifndef __ctkDICOMItemTreeModel_h
#define __ctkDICOMItemTreeModel_h

// Qt includes 
#include <QAbstractItemModel>
#include <QModelIndex>

// CTK includes
#include <ctkPimpl.h>

#include "CTKDICOMWidgetsExport.h"

class ctkDICOMItemTreeModelPrivate;

class CTK_DICOM_WIDGETS_EXPORT ctkDICOMItemTreeModel : public QAbstractItemModel
{
public:
  typedef QAbstractItemModel Superclass;
  explicit ctkDICOMItemTreeModel(QObject* parent=0);
  virtual ~ctkDICOMItemTreeModel();

  virtual QModelIndex index(int row, int column,
                            const QModelIndex &parent = QModelIndex()) const;
                            
  virtual QModelIndex parent(const QModelIndex &child) const;
    
  virtual Qt::ItemFlags flags(const QModelIndex &index) const;

  virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

  virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role = Qt::DisplayRole) const;

  virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
  virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;

private:
  CTK_DECLARE_PRIVATE(ctkDICOMItemTreeModel);
};

#endif

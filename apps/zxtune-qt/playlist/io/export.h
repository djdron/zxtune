/*
Abstract:
  Playlist export interfaces

Last changed:
  $Id$

Author:
  (C) Vitamin/CAIG/2001

  This file is a part of zxtune-qt application based on zxtune library
*/

#pragma once
#ifndef ZXTUNE_QT_PLAYLIST_IO_EXPORT_H_DEFINED
#define ZXTUNE_QT_PLAYLIST_IO_EXPORT_H_DEFINED

//local includes
#include "container.h"

class QString;
namespace Playlist
{
  namespace IO
  {
    class ExportCallback
    {
    public:
      virtual ~ExportCallback() {}

      virtual void Progress(unsigned percents) = 0;
      virtual bool IsCanceled() const = 0;
    };

    Error SaveXSPF(Container::Ptr container, const QString& filename, ExportCallback& cb);
  }
}

#endif //ZXTUNE_QT_PLAYLIST_IO_EXPORT_H_DEFINED

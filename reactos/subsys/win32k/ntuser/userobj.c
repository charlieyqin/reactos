/*
 *  ReactOS W32 Subsystem
 *  Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003 ReactOS Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
/* $Id: userobj.c,v 1.2 2003/05/18 17:16:17 ea Exp $
 *
 * COPYRIGHT:        See COPYING in the top level directory
 * PROJECT:          ReactOS kernel
 * PURPOSE:          USER Object manager interface definition
 * FILE:             subsys/win32k/ntuser/userobj.c
 * PROGRAMER:        Rex Jolliff (rex@lvcablemodem.com)
 *
 */

#undef WIN32_LEAN_AND_MEAN
#include <windows.h>
#define NDEBUG
#include <debug.h>
#include <ddk/ntddk.h>
#include <win32k/userobj.h>

static  LIST_ENTRY  UserObjectList;

PUSER_OBJECT FASTCALL USEROBJ_AllocObject (WORD size, WORD magic)
{
  PUSER_OBJECT_HEADER  newObject;

  newObject = ExAllocatePoolWithTag(PagedPool, 
                                    size + sizeof (USER_OBJECT_HEADER), 
                                    USER_OBJECT_TAG);
  if (newObject == 0)
  {
    return  0;
  }
  RtlZeroMemory(newObject, size + sizeof (USER_OBJECT_HEADER));

  newObject->magic = magic;
  ExInitializeFastMutex (&newObject->mutex);
  InsertTailList (&UserObjectList, &newObject->listEntry);

  return  UserObjectHeaderToBody (newObject);
}

BOOL FASTCALL USEROBJ_FreeObject (PUSER_OBJECT object, WORD magic)
{
  PUSER_OBJECT_HEADER  objectHeader;

  if (object == NULL)
  {
    return FALSE;
  }
  objectHeader = UserObjectBodyToHeader (object);
  if (objectHeader->magic != magic)
  {
    return  FALSE;
  }
  RemoveEntryList (&objectHeader->listEntry);
  ExFreePool (objectHeader);

  return  TRUE;
}

HUSEROBJ FASTCALL USEROBJ_PtrToHandle (PUSER_OBJECT object, WORD  magic)
{
  PUSER_OBJECT_HEADER  objectHeader;
  
  if (object == 0)
  {
    return  0;
  }
  objectHeader = UserObjectBodyToHeader (object);
  if (objectHeader->magic != magic)
  {
    return  0;
  }
  
  return  UserObjectHeaderToHandle(objectHeader);
}

PUSER_OBJECT FASTCALL USEROBJ_HandleToPtr (HUSEROBJ  handle, WORD  magic)
{
  PUSER_OBJECT_HEADER  objectHeader;

  if (handle == 0)
  {
    return  0;
  }
  objectHeader = UserObjectHandleToHeader (handle);
  if ((objectHeader->magic != magic) && 
      (magic != UO_MAGIC_DONTCARE))
  {
    return  0;
  }

  return  UserObjectHeaderToBody (objectHeader);
}

BOOL FASTCALL USEROBJ_LockObject (HUSEROBJ  objectHandle)
{
  PUSER_OBJECT_HEADER  objectHeader;

  if (objectHandle == 0)
  {
    return  FALSE;
  }
  objectHeader = UserObjectHandleToHeader (objectHandle);

  ExAcquireFastMutexUnsafe (&objectHeader->mutex);

  return  TRUE;
}

BOOL FASTCALL USEROBJ_UnlockObject (HUSEROBJ  objectHandle)
{
  PUSER_OBJECT_HEADER  objectHeader;

  if (objectHandle == 0)
  {
    return  FALSE;
  }
  objectHeader = UserObjectHandleToHeader (objectHandle);

  ExReleaseFastMutexUnsafe (&objectHeader->mutex);

  return  TRUE;
}

/* EOF */

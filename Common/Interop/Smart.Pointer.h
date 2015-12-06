/**
* Environs Smart Pointer Macros
* ------------------------------------------------------------------
 * Copyright (c) Chi-Tai Dang
 *
 * @author	Chi-Tai Dang
 * @version	1.0
 * @remarks
 *
 * This file is part of the Environs framework developed at the
 * Lab for Human Centered Multimedia of the University of Augsburg.
 * http://hcm-lab.de/environs
 *
 * Environ is free software; you can redistribute it and/or modify
 * it under the terms of the Eclipse Public License v1.0.
 * A copy of the license may be obtained at:
 * http://www.eclipse.org/org/documents/epl-v10.html
 * --------------------------------------------------------------------
*/
#pragma once
#ifndef INCLUDE_HCM_ENVIRONS_SMART_POINTER_H
#define INCLUDE_HCM_ENVIRONS_SMART_POINTER_H


#ifdef __cplusplus

#if !defined(CLI_CPP)

#	include <memory>
#	include <vector>
#	include <map>

#	define sp_reset(sp)				sp = 0
#	define sp_set_no_cli(d,s)		d = s
#	define sp_get(v)				v.get()
#	define sp_make(type)			std::make_shared < type > ();
#	define sp_make_arg(type,arg)	std::make_shared < type > ( arg );

#	define vp_size(v)				v->size()
#	define vp_sized(v)				v.size()
#	define vp_at(v,i)				v->at ( i )
#	define vp_atd(v,i)				v.at ( i )

#	define stdQueue(type)			std::queue < type >
#	define stdQueue_empty(q)		q.empty()
#	define stdQueue_front(q)		q.front()
#	define stdQueue_pop(q)			q.pop()
#	define stdQueue_push(q,e)		q.push(e)

#	define envArrayList				environs::ArrayList

#	define sp(type)					std::shared_ptr < type >

#	define sp_assign(type,name,val) sp(type) name ((type *)val)

#	define vctSize			
#	define vct(type)				std::vector < type >
#	define spv(type)				std::shared_ptr < std::vector < type > >
#	define spvc(type)				std::shared_ptr < std::vector < type > >

#	define vsp(type)				std::vector < std::shared_ptr < type > >

#	define svsp(type)				std::shared_ptr < std::vector < std::shared_ptr < type > > >

#	define msp(key,type)			std::map < key, std::shared_ptr < type > >
#	define smsp(key,type)			std::shared_ptr < std::map < key, std::shared_ptr < type > > >

#	define devListNew(type)			make_shared < vsp ( type ) > ()
#	define devList(type)			svsp(type)
#	define devListRef(type)			vsp(type) *
#	define DeviceListAppend(l,d)	l->push_back (d)
#	define DeviceListInsert(l,i,d)	l->insert ( l->begin () + vctSize i, d )
#	define DeviceListRemoveAt(l,i)	l->erase ( l->begin () + vctSize i )
#	define DeviceListClear(l)		l->clear ()

#	define ContainerClear(l)		l->clear ()
#	define ContainerCleard(l)		l.clear ()
#	define ContainerAppend(l,d)		l->push_back (d)
#	define ContainerAppendd(l,d)	l.push_back (d)
#	define ContainerInsert(l,i,d)	l->insert ( l->begin () + vctSize i, d )
#	define ContainerRemoveAt(l,i)	l->erase ( l->begin () + vctSize i )
#	define ContainerRemoveAtd(l,i)	l.erase ( l.begin () + vctSize i )
#	define ContainerRemove(l,i)		l->erase ( i )
#	define ContainerIfContains(l,i)	

#	define NLayerMapType(a,b)		smsp(a,b)
#	define NLayerVecType(a)			svsp(a)
#	define NLayerMapTypeObj(a,b)	msp(a,b)
#	define NLayerVecTypeObj(a)		vsp(a)
#	define NLayerListType(a)		std::shared_ptr < std::vector < a * > >
#	define NLayerListTypeObj(a)		std::vector < a * > 

#	define NLayerMapRaw(key,t)		std::map < key, t * >

#	define DeviceListItems						DeviceInfo    *
#	define DevicesToPlatform(devHead)			&devHead->device
#	define DevicesPlatformCount(devHead,devs)	devHead->header.deviceCount
#	define DevicesPlatformAt(devs,pos)			devices + pos

#	define DisplayPropsToPlatform(d,i)			d

#	define up(type)					std::unique_ptr < type >

#	define sp_autorelease(type,obj)	std::shared_ptr < type > ( obj, environs::EnvironsDisposer )

#	define AppEnvRemove(a,b)		
#	define AppEnvUpdate(a,b)

// Not supported by vs 2010
            //template<typename T>
            //using spd = std::shared_ptr<T>;
#else

// CLI / CPP

using namespace System;
using namespace System::Collections::Generic;

using System::Collections::Generic::Queue;
using System::Collections::ArrayList;
using System::Collections::Generic::List;

using System::Collections::ObjectModel::ObservableCollection;


#	define stdQueue(type)			System::Collections::Generic::Queue < type >
#	define stdQueue_empty(q)		(q.Count == 0)
#	define stdQueue_front(q)		q.Dequeue()
#	define stdQueue_pop(q)			
#	define stdQueue_push(q,e)		q.Enqueue(e)

#	define envArrayList				System::Collections::ArrayList

#	define up(type)					type ^

#	define sp(type)					type ^
#	define sp_set_no_cli(d,s)
#	define sp_reset(sp)		
#	define sp_get(v)				v
#	define sp_make(type)			gcnew type ()
#	define sp_make_arg(type,arg)	gcnew type ( arg );

#	define spv(type)				ArrayList ^
#	define spvc(type)				ObservableCollection<type> ^

#	define svsp(type)				cliext::vector < type ^ > ^
#	define vsp(type)				cliext::vector < type ^ >

#	define vctSize					(int)
#	define vct(type)				cliext::vector < type >

#	define vp_size(v)				(size_t) v->Count
#	define vp_sized(v)				(size_t) v.Count

#	define vp_at(v,i)				v[vctSize i]
#	define vp_atd(v,i)				v[vctSize i]

#	define msp(key,type)			cliext::map < key, type ^ >
#	define smsp(key,type)			cliext::map < key, type ^ > ^

#	define sp_assign(type,name,val) type ^ name = ((type ^ )val)
#	define devListNew(type)			gcnew ObservableCollection<type ^> ()

#	define devList(type)			ObservableCollection<type ^> ^
#	define devListRef(type)			devList(type)
#	define ContainerClear(l)		l->Clear ()
#	define ContainerCleard(l)		l.Clear ()

#	define DeviceListAppend(l,d)	DeviceListAppendFunc ( l, d )
#	define DeviceListInsert(l,i,d)	DeviceListInsertFunc ( l, d, i )
#	define DeviceListRemoveAt(l,i)	DeviceListRemoveAtFunc ( l, i )
#	define DeviceListClear(l)		DeviceListClearFunc ( l )


// TODO need to be done in UI thread
#	define ContainerAppend(l,d)		l->Add (d)
#	define ContainerAppendd(l,d)	l.Add (d)
#	define ContainerInsert(l,i,d)	l->Insert ( i, d )
#	define ContainerRemoveAt(l,i)	l->RemoveAt ( i )
#	define ContainerRemoveAtd(l,i)	l.RemoveAt ( i )
#	define ContainerRemove(l,i)		if (l->ContainsKey(i)) l->Remove ( i )
#	define ContainerIfContains(l,i)	if (l->ContainsKey(i))

#	define NLayerMapType(a,b)		Dictionary<a, b ^> ^
#	define NLayerVecType(a)			List<a ^> ^
#	define NLayerMapTypeObj(a,b)	Dictionary<a, b ^> 
#	define NLayerVecTypeObj(a)		List<a ^> 
#	define NLayerListType(a)		List<a ^> ^
#	define NLayerListTypeObj(a)		List<a ^> 

#	define NLayerMapRaw(key,t)		Dictionary<key, t ^> 

#	define DeviceListItems						cli::array < DeviceInstance ^ > ^
#	define DevicesToPlatform(devHead)			BuildDeviceInfoList(api->hEnvirons, IntPtr(devHead), true)
#	define DevicesPlatformCount(devHead,devs)	devs->Length
#	define DevicesPlatformAt(devs,pos)			devices [ pos ]->info_

#	define DisplayPropsToPlatform(d,i)			BuildDeviceDisplayProps(d,i)

#ifdef CLI_PS
#	define AppEnvRemove(a,b)			ApplicationEnvironment::RemoveAppEnv ( a, b );
#	define AppEnvUpdate(a,b)			ApplicationEnvironment::UpdateAppEnv ( a, b  );
#else
#	define AppEnvRemove(a,b)		
#	define AppEnvUpdate(a,b)
#endif

#endif

#else
// No cpp, so.. undefine the macros

#	define sp(type)

#	define stdQueue(type)
#	define vct(type)
#	define spv(type)
#	define spvc(type)
#	define vsp(type)
#	define svsp(type)

#	define msp(key,type)
#	define smsp(key,type)
#	define spget(v)	
#	define vp_size(v)		

#	define ContainerDevList(type)	
#	define ContainerRef(type)		
#	define ContainerClear(l)	
#	define ContainerAppend(d)

#	define up(type)

#	define sp_autorelease(type,obj)

#endif

#endif // INCLUDE_HCM_ENVIRONS_SMART_POINTER_H
// AVSImagePaint3.cpp : Implementation of DLL Exports.

#include "stdafx.h"
#include "resource.h"

// The module attribute causes DllMain, DllRegisterServer and DllUnregisterServer to be automatically implemented for you
[module(dll, 
		uuid = "{084DA982-7EA0-4af3-A44D-97259DFBEC6F}", 
		name = "AVSImagePaint3", 
		helpstring = "AVSImagePaint 3",
		resource_name = "IDR_AVSIMAGEPAINT3")];

#include "ImagePaint3.h"
#include "ImageTextStatic3.h"
#include "ImageTextDynamic3.h"
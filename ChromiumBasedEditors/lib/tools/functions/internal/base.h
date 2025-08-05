#pragma once
#include "../../tools.h"

#ifndef JSON_HAS_CPP_11
#define JSON_HAS_CPP_11
#endif

#include "./json.hpp"
using namespace nlohmann;
#include "../../../../../../core/DesktopEditor/doctrenderer/docbuilder.h"
using namespace NSDoctRenderer;

#include "../../../../../../core/DesktopEditor/common/File.h"
#include "../../../../../../core/Common/OfficeFileFormats.h"

static void initBuilder(CDocBuilder* builder)
{
	std::wstring allFontsJS = CAITools::getInstance().getFontsDirectory() + L"/AllFonts.js";
	builder->SetProperty("--all-fonts-path", allFontsJS.c_str());
}

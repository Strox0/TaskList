#pragma once
#ifndef IMGUIDATECHOOSER_H_
#define IMGUIDATECHOOSER_H_

#ifndef IMGUI_API
#include <imgui.h>
#endif //IMGUI_API

// USAGE
/*
#include "imguidatechooser.h"
//#include <time.h>   // mandatory when implementing ImGui::TestDateChooser() yourself in your code

// inside a ImGui::Window:
ImGui::TestDateChooser();
*/

struct tm;  // defined in <time.h>

namespace ImGui {

	// Some example of date formats: "%d/%m/%Y" "%A %d %b %Y"
	IMGUI_API bool DateChooser(const char* label, tm& dateOut, const char* dateFormat = "%d/%m/%Y", bool closeWhenMouseLeavesIt = true, bool* pSetStartDateToDateOutThisFrame = NULL, const char* leftArrow = "<", const char* rightArrow = ">", const char* upArrowString = "   ^   ", const char* downArrowString = "   v   ");
	IMGUI_API bool DateChooserPopup(const char* label, tm& dateOut, bool* pSetStartDateToDateOutThisFrame = NULL, const char* leftArrow = "<", const char* rightArrow = ">");
	// Some helper methods (note that I cannot use tm directly in this hader file, so we can't initialize a static date directly with these methods)
	IMGUI_API void SetDateZero(tm* date);
	IMGUI_API void SetDateToday(tm* date);

	IMGUI_API bool TimeChooserPopup(const char* label, tm& timeOut, bool* pSetStartTimeToTimeOutThisFrame = NULL, const char* upArrow = "^", const char* downArrow = "v");

	IMGUI_API void TestDateChooser(const char* dateFormat = "%d/%m/%Y", bool closeWhenMouseLeavesIt = true, const char* leftArrow = "<", const char* rightArrow = ">", const char* upArrowString = "   ^   ", const char* downArrowString = "   v   ");


} // namespace ImGui



#endif


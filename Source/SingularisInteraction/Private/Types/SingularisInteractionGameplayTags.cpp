/* ====================================================================== *
 * SingularisInteractionGameplayTags.cpp                                  *
 * ====================================================================== *
 * SPDX-License-Identifier: MIT                                           *
 * SPDX-FileCopyrightText: 2026 TrifingZW <TrifingZW@gmail.com>           *
 *                                                                        *
 * Copyright (c) 2026 TrifingZW. All Rights Reserved.                     *
 * Created: 2026/01/22 | Author: TrifingZW                                *
 * Licensed under MIT License                                             *
 *                                                                        *
 * Permission is hereby granted, free of charge, to any person obtaining  *
 * a copy of this software and associated documentation files (the        *
 * "Software"), to deal in the Software without restriction, including    *
 * without limitation the rights to use, copy, modify, merge, publish,    *
 * distribute, sublicense, and/or sell copies of the Software, and to     *
 * permit persons to whom the Software is furnished to do so, subject to  *
 * the following conditions:                                              *
 *                                                                        *
 * The above copyright notice and this permission notice shall be         *
 * included in all copies or substantial portions of the Software.        *
 *                                                                        *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        *
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. *
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   *
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   *
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      *
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 *
 * ====================================================================== */

#include "Types/SingularisInteractionGameplayTags.h"

#include <NativeGameplayTags.h>

UE_DEFINE_GAMEPLAY_TAG_COMMENT(SingularisInteraction, "Singularis.Interaction", "交互域根标签")
UE_DEFINE_GAMEPLAY_TAG_COMMENT(SingularisInteraction_Intent, "Singularis.Interaction.Intent", "意图域根标签")
UE_DEFINE_GAMEPLAY_TAG_COMMENT(
	SingularisInteraction_Intent_Press,
	"Singularis.Interaction.Intent.Press",
	"意图：主张，会话建立的唯一触发"
)
UE_DEFINE_GAMEPLAY_TAG_COMMENT(SingularisInteraction_Intent_Release, "Singularis.Interaction.Intent.Release", "意图：撤回")
UE_DEFINE_GAMEPLAY_TAG_COMMENT(SingularisInteraction_Intent_Value, "Singularis.Interaction.Intent.Value", "意图：值")
UE_DEFINE_GAMEPLAY_TAG_COMMENT(SingularisInteraction_Event, "Singularis.Interaction.Event", "事件域根标签")
UE_DEFINE_GAMEPLAY_TAG_COMMENT(SingularisInteraction_Event_Begin, "Singularis.Interaction.Event.Begin", "事件：会话建立")
UE_DEFINE_GAMEPLAY_TAG_COMMENT(
	SingularisInteraction_Event_Progress,
	"Singularis.Interaction.Event.Progress",
	"事件：会话推进，唯一连续事件"
)
UE_DEFINE_GAMEPLAY_TAG_COMMENT(SingularisInteraction_Event_Settle, "Singularis.Interaction.Event.Settle", "事件：会话定格")
UE_DEFINE_GAMEPLAY_TAG_COMMENT(SingularisInteraction_Event_Complete, "Singularis.Interaction.Event.Complete", "事件：会话达成")
UE_DEFINE_GAMEPLAY_TAG_COMMENT(
	SingularisInteraction_Event_Cancel,
	"Singularis.Interaction.Event.Cancel",
	"事件：会话中止，含准入拒绝反馈"
)
UE_DEFINE_GAMEPLAY_TAG_COMMENT(SingularisInteraction_Response, "Singularis.Interaction.Response", "响应域根标签")
UE_DEFINE_GAMEPLAY_TAG_COMMENT(
	SingularisInteraction_Response_Default,
	"Singularis.Interaction.Response.Default",
	"默认响应标签"
)

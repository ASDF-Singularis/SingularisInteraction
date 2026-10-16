/* ====================================================================== *
 * SingularisInteractionSessionDrag.cpp                                   *
 * ====================================================================== *
 * SPDX-License-Identifier: MIT                                           *
 * SPDX-FileCopyrightText: 2026 TrifingZW <TrifingZW@gmail.com>           *
 *                                                                        *
 * Copyright (c) 2026 TrifingZW. All Rights Reserved.                     *
 * Created: 2026/10/13 | Author: TrifingZW                                *
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

#include "Sessions/SingularisInteractionSessionDrag.h"

#include <Math/UnrealMathUtility.h>

#include "Types/SingularisInteractionGameplayTags.h"

void USingularisInteractionSessionDrag::Begin_Implementation(const FSingularisInteractionSessionContext& Context)
{
	// 1) 复位绝对进度并广播初始态
	Progress = 0.0f;
	EmitSessionEvent(SingularisInteraction_Event_Progress, FSingularisInteractionValue(Progress), FInstancedStruct());
}

void USingularisInteractionSessionDrag::ReceiveIntent_Implementation(
	const FSingularisInteractionSessionContext& Context,
	const FSingularisInteractionIntent& Intent
)
{
	// 1) 撤回：定格当前角度，并按配置达成
	if (Intent.IntentTag == SingularisInteraction_Intent_Release)
	{
		EmitSessionEvent(SingularisInteraction_Event_Settle, FSingularisInteractionValue(Progress), FInstancedStruct());

		if (bCompleteOnRelease)
			CompleteSession(FInstancedStruct());

		return;
	}

	// 2) 仅响应连续值意图：按带符号标量绝对跟踪
	if (Intent.IntentTag != SingularisInteraction_Intent_Value)
		return;

	Progress = FMath::Clamp(Progress + Intent.Value.GetScalar() * Sensitivity, 0.0f, 1.0f);
	EmitSessionEvent(SingularisInteraction_Event_Progress, FSingularisInteractionValue(Progress), FInstancedStruct());

	// 3) 拖满即达成；拖回起点按配置中止
	if (Progress >= 1.0f)
	{
		CompleteSession(FInstancedStruct());
		return;
	}

	if (bCancelAtZero && Progress <= 0.0f)
		CancelSession(FInstancedStruct());
}

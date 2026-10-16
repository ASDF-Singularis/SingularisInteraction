/* ====================================================================== *
 * SingularisInteractionSessionAxis.cpp                                   *
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

#include "Sessions/SingularisInteractionSessionAxis.h"

#include <Math/UnrealMathUtility.h>

#include "Types/SingularisInteractionGameplayTags.h"

void USingularisInteractionSessionAxis::Begin_Implementation(const FSingularisInteractionSessionContext& Context)
{
	AccumulatedValue = 0.0f;
}

void USingularisInteractionSessionAxis::ReceiveIntent_Implementation(
	const FSingularisInteractionSessionContext& Context,
	const FSingularisInteractionIntent& Intent
)
{
	// 1) 仅响应连续值意图
	if (Intent.IntentTag != SingularisInteraction_Intent_Value)
		return;

	// 2) 按模式累积量值
	switch (AxisMode)
	{
	case ESingularisInteractionAxisMode::Integral:
		AccumulatedValue += Intent.Value.GetScalar();
		break;
	case ESingularisInteractionAxisMode::Peak:
		AccumulatedValue = FMath::Max(AccumulatedValue, Intent.Value.GetMagnitude());
		break;
	default:
		break;
	}

	// 3) 广播进度，达到阈值即完成
	const float Alpha = FMath::Clamp(AccumulatedValue / FMath::Max(Threshold, 0.01f), 0.0f, 1.0f);
	EmitSessionEvent(SingularisInteraction_Event_Progress, FSingularisInteractionValue(Alpha), FInstancedStruct());

	if (Alpha >= 1.0f)
		CompleteSession(FInstancedStruct());
}

/* ====================================================================== *
 * SingularisInteractionSessionHold.h                                     *
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

#pragma once

#include <CoreMinimal.h>

#include "Objects/SingularisInteractionSession.h"
#include "SingularisInteractionSessionHold.generated.h"

/**
 * 引力奇点长按交互会话。
 *
 * 按住达到时长即达成；提前撤回默认取消，亦可配置为进度衰减
 * （松手后按衰减率回落，归零即中止——蓄力类交互）。
 */
UCLASS(
	NotBlueprintable,
	BlueprintType,
	ClassGroup = ("Singularis"),
	meta = (DisplayName = "引力奇点长按交互会话")
)
class SINGULARISINTERACTION_API USingularisInteractionSessionHold : public USingularisInteractionSession
{
	GENERATED_BODY()

public:
#pragma region Parameter

	/** 长按时长（秒） */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "引力奇点长按会话",
		meta = (DisplayName = "长按时长", ClampMin = "0.01", ForceUnits = "s")
	)
	float HoldDuration = 1.5f;

	/** 提前撤回是否立即取消 */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "引力奇点长按会话",
		meta = (DisplayName = "撤回即取消")
	)
	bool bCancelOnRelease = true;

	/** 进度衰减率（0..1 每秒）：撤回不取消时按此回落，归零即中止；0 为不衰减 */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "引力奇点长按会话",
		meta = (DisplayName = "进度衰减率", ClampMin = "0.0", ClampMax = "1.0")
	)
	float DecayRate = 0.0f;

#pragma endregion

private:
#pragma region State

	/** 是否已撤回（衰减分支入口） */
	bool bReleased = false;

	/** 内部进度量值 */
	float InternalAlpha = 0.0f;

#pragma endregion

public:
#pragma region SPI

	virtual void Begin_Implementation(const FSingularisInteractionSessionContext& Context) override;
	virtual void ReceiveIntent_Implementation(
		const FSingularisInteractionSessionContext& Context,
		const FSingularisInteractionIntent& Intent
	) override;
	virtual void Tick_Implementation(const FSingularisInteractionSessionContext& Context, float DeltaTime) override;

#pragma endregion
};

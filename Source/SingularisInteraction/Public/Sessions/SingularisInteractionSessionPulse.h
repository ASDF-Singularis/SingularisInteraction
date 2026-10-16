/* ====================================================================== *
 * SingularisInteractionSessionPulse.h                                    *
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
#include "SingularisInteractionSessionPulse.generated.h"

/**
 * 引力奇点连点交互会话。
 *
 * 窗口期内累计主张达到次数即达成；窗口自最近一次主张起算，超时即中止。
 * 首次主张由会话建立承载（Begin 即第一声），后续主张经意图转发计数。
 */
UCLASS(
	NotBlueprintable,
	BlueprintType,
	ClassGroup = ("Singularis"),
	meta = (DisplayName = "连点")
)
class SINGULARISINTERACTION_API USingularisInteractionSessionPulse : public USingularisInteractionSession
{
	GENERATED_BODY()

public:
#pragma region Parameter

	/** 连点完成次数（至少 2 次） */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "引力奇点连点会话",
		meta = (DisplayName = "连点完成次数", ClampMin = "2")
	)
	int32 PulseCount = 3;

	/** 连点窗口期（秒），自最近一次主张起算 */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "引力奇点连点会话",
		meta = (DisplayName = "连点窗口期", ClampMin = "0.01", ForceUnits = "s")
	)
	float PulseWindow = 0.8f;

#pragma endregion

private:
#pragma region State

	/** 已计主张次数 */
	int32 PulseCounter = 0;

	/** 最近一次主张的会话时刻（秒） */
	float LastPulseElapsed = 0.0f;

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

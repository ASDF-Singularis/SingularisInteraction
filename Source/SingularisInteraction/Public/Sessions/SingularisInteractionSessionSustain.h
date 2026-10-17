/* ====================================================================== *
 * SingularisInteractionSessionSustain.h                                  *
 * ====================================================================== *
 * SPDX-License-Identifier: MIT                                           *
 * SPDX-FileCopyrightText: 2026 TrifingZW <TrifingZW@gmail.com>           *
 *                                                                        *
 * Copyright (c) 2026 TrifingZW. All Rights Reserved.                     *
 * Created: 2026/10/18 | Author: TrifingZW                                *
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
#include "SingularisInteractionSessionSustain.generated.h"

/**
 * 引力奇点维持交互会话。
 *
 * 会话寿命即按压寿命：按下建立，松手即结束（达成）。无参数、不产生中间进度，
 * 按压期间的表现由响应自行承担；需要按满才成功时使用长按会话，
 * 需要按住拖动控制量值时使用拖动会话。
 */
UCLASS(
	NotBlueprintable,
	BlueprintType,
	ClassGroup = ("Singularis"),
	meta = (DisplayName = "维持")
)
class SINGULARISINTERACTION_API USingularisInteractionSessionSustain : public USingularisInteractionSession
{
	GENERATED_BODY()

public:
#pragma region SPI

	virtual void ReceiveIntent_Implementation(
		const FSingularisInteractionSessionContext& Context,
		const FSingularisInteractionIntent& Intent
	) override;

#pragma endregion
};

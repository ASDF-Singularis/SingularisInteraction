/* ====================================================================== *
 * SingularisInteractionBehavior.h                                        *
 * ====================================================================== *
 * SPDX-License-Identifier: MIT                                           *
 * SPDX-FileCopyrightText: 2026 TrifingZW <TrifingZW@gmail.com>           *
 *                                                                        *
 * Copyright (c) 2026 TrifingZW. All Rights Reserved.                     *
 * Created: 2026/01/30 | Author: TrifingZW                                *
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
#include <UObject/Object.h>

#include "Types/SingularisInteractionBehaviorType.h"
#include "SingularisInteractionBehavior.generated.h"

/**
 * 引力奇点交互行为。
 *
 * 响应交互组件的启用与悬浮状态变化执行副作用，由交互组件以子对象形式持有。
 * 子类覆写 SPI 实现具体表现；行为不参与交互触发管线。
 */
UCLASS(Abstract, Blueprintable, EditInlineNew, CollapseCategories)
class SINGULARISINTERACTION_API USingularisInteractionBehavior : public UObject
{
	GENERATED_BODY()

public:
#pragma region UObject Interface

	virtual UWorld* GetWorld() const override;

#pragma endregion

#pragma region SPI

	/** 交互启用时执行。 */
	UFUNCTION(
		BlueprintNativeEvent,
		BlueprintCallable,
		Category = "引力奇点交互行为|SPI",
		meta = (DisplayName = "Enabled")
	)
	void Enabled(const FSingularisInteractionBehaviorContext& Context);

	/** 交互禁用时执行。 */
	UFUNCTION(
		BlueprintNativeEvent,
		BlueprintCallable,
		Category = "引力奇点交互行为|SPI",
		meta = (DisplayName = "Disabled")
	)
	void Disabled(const FSingularisInteractionBehaviorContext& Context);

	/** 交互悬浮时执行。 */
	UFUNCTION(
		BlueprintNativeEvent,
		BlueprintCallable,
		Category = "引力奇点交互行为|SPI",
		meta = (DisplayName = "Hovered")
	)
	void Hovered(const FSingularisInteractionBehaviorContext& Context);

	/** 交互未悬浮时执行。 */
	UFUNCTION(
		BlueprintNativeEvent,
		BlueprintCallable,
		Category = "引力奇点交互行为|SPI",
		meta = (DisplayName = "Unhovered")
	)
	void Unhovered(const FSingularisInteractionBehaviorContext& Context);

#pragma endregion
};

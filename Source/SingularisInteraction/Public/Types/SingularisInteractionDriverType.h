/* ====================================================================== *
 * SingularisInteractionDriverType.h                                      *
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
#include <GameplayTagContainer.h>

#include "SingularisInteractionDriverType.generated.h"

class UInputAction;

/**
 * 引力奇点交互输入绑定。
 *
 * 增强输入驱动器的配置单元：把输入动作映射到策略标签，
 * 可选携带连续值（Triggered → Intent.Value）。
 */
USTRUCT(BlueprintType)
struct SINGULARISINTERACTION_API FSingularisInteractionInputBinding
{
	GENERATED_BODY()

	/** 输入动作 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UInputAction> InputAction = nullptr;

	/** 动作映射的策略标签 */
	UPROPERTY(
		EditAnywhere,
		BlueprintReadWrite,
		meta = (
			Categories = "Singularis.Interaction.Strategy",
			ForceSelection = "true"
		)
	)
	FGameplayTag StrategyTag{};

	/** 是否携带连续值（Triggered → Intent.Value，轴交互开启） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCarryValue = false;
};

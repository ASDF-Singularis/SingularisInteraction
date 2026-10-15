/* ====================================================================== *
 * SingularisInteractionEnhancedInputDriver.h                             *
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
#include <InputActionValue.h>

#include "Objects/SingularisInteractionDriver.h"
#include "Types/SingularisInteractionDriverType.h"
#include "Types/SingularisInteractionValueType.h"
#include "SingularisInteractionEnhancedInputDriver.generated.h"

class UInputAction;
class UInputMappingContext;

/**
 * 引力奇点交互连续值跟踪。
 *
 * 连续值意图的节流与死区判据，抑制每帧提交噪声。
 */
struct FSingularisInteractionValueTracking
{
	/** 最近提交量值 */
	float Value = 0.0f;

	/** 最近提交时刻（世界秒） */
	float Time = 0.0f;
};

/**
 * 引力奇点增强输入驱动器。
 *
 * 玩家路径的输入源适配：把增强输入的动作事件翻译为中性意图
 * （Started → Press，Completed / Canceled → Release，Triggered → Value）。
 * 输入侧塑形（Hold / Pulse 等触发器与修改器）仍由增强输入资产承担，
 * 属于手感预筛的 UX 手段；玩法时序由逻辑端会话推进，二者不重复计时。
 */
UCLASS(Blueprintable, EditInlineNew, CollapseCategories)
class SINGULARISINTERACTION_API USingularisInteractionEnhancedInputDriver : public USingularisInteractionDriver
{
	GENERATED_BODY()

public:
#pragma region Parameter

	/** 输入映射上下文 */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "引力奇点增强输入驱动器",
		meta = (DisplayName = "输入映射上下文")
	)
	UInputMappingContext* InputMappingContext = nullptr;

	/** 输入优先级 */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "引力奇点增强输入驱动器",
		meta = (DisplayName = "输入优先级")
	)
	int32 InputPriority = 10;

	/** 输入绑定集：动作 → 策略标签 */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "引力奇点增强输入驱动器",
		meta = (DisplayName = "输入绑定集")
	)
	TArray<FSingularisInteractionInputBinding> InputBindings{};

	/** 连续值提交间隔（秒），节流上限 */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "引力奇点增强输入驱动器",
		meta = (DisplayName = "连续值提交间隔", ClampMin = "0.0", ForceUnits = "s")
	)
	float ValueSubmitInterval = 1.0f / 30.0f;

	/** 连续值死区：低于此变化量且未达间隔时不提交 */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "引力奇点增强输入驱动器",
		meta = (DisplayName = "连续值死区", ClampMin = "0.0")
	)
	float ValueDeadband = 0.05f;

#pragma endregion

private:
#pragma region State

	/** 连续值跟踪表，键为策略标签 */
	TMap<FGameplayTag, FSingularisInteractionValueTracking> ValueTracking{};

#pragma endregion

public:
#pragma region Constructors

	USingularisInteractionEnhancedInputDriver();

#pragma endregion

#pragma region SPI

	virtual void InitializeDriver_Implementation(USingularisInteractorComponent* InInteractorComponent) override;
	virtual void DeinitializeDriver_Implementation() override;
	virtual void OnTargetChanged_Implementation(
		USingularisInteractionComponent* OldTarget,
		USingularisInteractionComponent* NewTarget
	) override;

#pragma endregion

private:
#pragma region Callback

	/** 输入按下回调：提交主张意图 */
	void HandleActionStarted(const FInputActionValue& ActionValue, FGameplayTag StrategyTag);

	/** 输入松开或强制取消回调：提交撤回意图 */
	void HandleActionReleased(const FInputActionValue& ActionValue, FGameplayTag StrategyTag);

	/** 输入持续回调：节流提交连续值意图 */
	void HandleActionValue(const FInputActionValue& ActionValue, FGameplayTag StrategyTag);

#pragma endregion

#pragma region Internal Function

	/** 提交意图至宿主交互者组件 */
	void SubmitIntent(FGameplayTag StrategyTag, FGameplayTag IntentTag, const FSingularisInteractionValue& Value) const;

	/** 依据当前目标状态挂载或卸载输入映射上下文 */
	void RefreshInputMappingContext() const;

#pragma endregion
};

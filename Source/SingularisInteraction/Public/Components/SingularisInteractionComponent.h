/* ====================================================================== *
 * SingularisInteractionComponent.h                                       *
 * ====================================================================== *
 * SPDX-License-Identifier: MIT                                           *
 * SPDX-FileCopyrightText: 2025 TrifingZW <TrifingZW@gmail.com>           *
 *                                                                        *
 * Copyright (c) 2025 TrifingZW. All Rights Reserved.                     *
 * Created: 2025/11/04 | Author: TrifingZW                                *
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
#include <Components/ActorComponent.h>

#include "Types/SingularisInteractionComponentType.h"
#include "SingularisInteractionComponent.generated.h"

struct FInputActionValue;
class USingularisInteractionBehaviorStrategy;
class USingularisInteractionStrategy;

#pragma region 委托签名

/** 开始交互时广播 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInteractionSignature);

/** 交互启用时广播 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInteractionEnableSignature);

/** 交互禁用时广播 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInteractionDisableSignature);

/** 交互悬浮时广播 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInteractionHoverSignature);

/** 交互未悬浮时广播 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInteractionUnhoverSignature);

#pragma endregion

/**
 * 引力奇点交互组件。
 *
 * 承载单个可交互目标的交互语义：维护启用与悬浮状态，按策略标签分发交互管线，
 * 并在状态变化时驱动行为策略执行副作用。
 */
UCLASS(
	Blueprintable,
	BlueprintType,
	ClassGroup = ("Singularis"),
	meta = (BlueprintSpawnableComponent, DisplayName = "引力奇点交互组件")
)
class SINGULARISINTERACTION_API USingularisInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
#pragma region Parameter

	/** 交互目标组件引用，用于在映射子系统登记本组件的归属 */
	UPROPERTY(
		EditAnywhere,
		BlueprintReadWrite,
		Category = "引力奇点交互组件",
		meta = (DisplayName = "交互目标", UseComponentPicker, AllowedClasses = "/Script/Engine.SceneComponent")
	)
	TArray<FComponentReference> TargetComponentReferences{};

	/** 交互策略管线映射，键为策略标签，值为按序执行的策略管线 */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "引力奇点交互组件",
		meta = (
			DisplayName = "交互策略管线映射",
			Categories = "Singularis.Interaction.Strategy",
			ForceSelection = "true"
		)
	)
	TMap<FGameplayTag, FSingularisInteractionStrategyPipeline> InteractionStrategyPipelineMapping{};

	/** 交互行为策略集，响应启用、禁用、悬浮与未悬浮状态变化 */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "引力奇点交互组件",
		meta = (DisplayName = "交互行为策略集")
	)
	TArray<FSingularisInteractionBehaviorStrategyEntry> InteractionBehaviorStrategies{};

#pragma endregion

#pragma region Event Dispatcher

	/** 开始交互时广播 */
	UPROPERTY(
		BlueprintAssignable,
		Category = "引力奇点交互组件|事件分发器",
		meta = (DisplayName = "开始交互时触发")
	)
	FOnInteractionSignature OnInteractionEvent{};

	/** 交互启用时广播 */
	UPROPERTY(
		BlueprintAssignable,
		Category = "引力奇点交互组件|事件分发器",
		meta = (DisplayName = "交互启用时触发")
	)
	FOnInteractionEnableSignature OnInteractionEnableEvent{};

	/** 交互禁用时广播 */
	UPROPERTY(
		BlueprintAssignable,
		Category = "引力奇点交互组件|事件分发器",
		meta = (DisplayName = "交互禁用时触发")
	)
	FOnInteractionDisableSignature OnInteractionDisableEvent{};

	/** 交互悬浮时广播 */
	UPROPERTY(
		BlueprintAssignable,
		Category = "引力奇点交互组件|事件分发器",
		meta = (DisplayName = "交互悬浮时触发")
	)
	FOnInteractionHoverSignature OnInteractionHoverEvent{};

	/** 交互未悬浮时广播 */
	UPROPERTY(
		BlueprintAssignable,
		Category = "引力奇点交互组件|事件分发器",
		meta = (DisplayName = "交互未悬浮时触发")
	)
	FOnInteractionUnhoverSignature OnInteractionUnhoverEvent{};

#pragma endregion

private:
#pragma region State

	/** 当前是否启用 */
	bool bIsEnabled = true;

	/** 当前是否被悬浮 */
	bool bIsHovered = false;

#pragma endregion

public:
#pragma region Constructors

	USingularisInteractionComponent();

#pragma endregion

#pragma region ActorComponent Interface

	virtual void BeginPlay() override;

#pragma endregion

#pragma region API

	/**
	 * 当前是否启用。
	 *
	 * @return 启用时返回 true。
	 */
	UFUNCTION(
		BlueprintPure,
		Category = "引力奇点交互组件|API",
		meta = (DisplayName = "Enabled")
	)
	bool Enabled() const { return bIsEnabled; }

	/**
	 * 当前是否被悬浮。
	 *
	 * @return 悬浮时返回 true。
	 */
	UFUNCTION(
		BlueprintPure,
		Category = "引力奇点交互组件|API",
		meta = (DisplayName = "Hovered")
	)
	bool Hovered() const { return bIsHovered; }

	/** 设置启用状态。状态未变化时幂等返回，变化后广播事件并驱动行为策略 */
	UFUNCTION(
		BlueprintCallable,
		Category = "引力奇点交互组件|API",
		meta = (DisplayName = "SetEnabled")
	)
	void SetEnabled(bool IsEnabled);

	/** 设置悬浮状态。状态未变化时幂等返回，变化后广播事件并驱动行为策略 */
	UFUNCTION(
		BlueprintCallable,
		Category = "引力奇点交互组件|API",
		meta = (DisplayName = "SetHovered")
	)
	void SetHovered(bool IsHovered);

#pragma endregion

#pragma region SPI

	/**
	 * 以指定交互策略标签触发一次交互。
	 *
	 * 仅服务器权威有效；按标签层级匹配策略管线并依次执行。
	 *
	 * @param StrategyTag 触发的交互策略标签。
	 * @param PlayerController 发起交互的玩家控制器。
	 * @param InputActionValue 触发交互的输入值。
	 */
	UFUNCTION(
		BlueprintCallable,
		BlueprintAuthorityOnly,
		Category = "引力奇点交互组件|SPI",
		meta = (DisplayName = "TryInteraction")
	)
	void TryInteraction(
		const FGameplayTag& StrategyTag,
		APlayerController* PlayerController,
		const FInputActionValue& InputActionValue
	);

#pragma endregion

private:
#pragma region Internal Function

	/** 向网络子系统登记策略子对象以参与复制 */
	void RegisterInteractionSubObjects();

#pragma endregion
};

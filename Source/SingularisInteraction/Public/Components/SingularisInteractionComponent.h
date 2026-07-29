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

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInteractionSignature);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInteractionEnableSignature);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInteractionDisableSignature);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInteractionHoverSignature);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInteractionUnhoverSignature);

#pragma endregion

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

	UPROPERTY(
		EditAnywhere,
		BlueprintReadWrite,
		Category = "SingularisInteraction|引力奇点交互|引用",
		meta = (DisplayName = "交互目标组件引用", UseComponentPicker, AllowedClasses = "/Script/Engine.SceneComponent")
	)
	TArray<FComponentReference> TargetComponentReferences{};

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "SingularisInteraction|引力奇点交互|策略",
		meta = (
			DisplayName = "交互策略管线映射",
			Categories = "Singularis.Interaction.Strategy",
			ForceSelection = "true"
		)
	)
	TMap<FGameplayTag, FSingularisInteractionStrategyPipeline> InteractionStrategyPipelineMapping{};

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "SingularisInteraction|引力奇点交互|策略",
		meta = (DisplayName = "交互行为策略集")
	)
	TArray<FSingularisInteractionBehaviorStrategyEntry> InteractionBehaviorStrategies{};

#pragma endregion

#pragma region 事件分发器

	UPROPERTY(
		BlueprintAssignable,
		Category = "SingularisInteraction|引力奇点交互|事件分发器",
		meta = (DisplayName = "开始交互时触发")
	)
	FOnInteractionSignature OnInteractionEvent{};

	UPROPERTY(
		BlueprintAssignable,
		Category = "SingularisInteraction|引力奇点交互|事件分发器",
		meta = (DisplayName = "交互启用时触发")
	)
	FOnInteractionEnableSignature OnInteractionEnableEvent{};

	UPROPERTY(
		BlueprintAssignable,
		Category = "SingularisInteraction|引力奇点交互|事件分发器",
		meta = (DisplayName = "交互禁用时触发")
	)
	FOnInteractionDisableSignature OnInteractionDisableEvent{};

	UPROPERTY(
		BlueprintAssignable,
		Category = "SingularisInteraction|引力奇点交互|事件分发器",
		meta = (DisplayName = "交互悬浮时触发")
	)
	FOnInteractionHoverSignature OnInteractionHoverEvent{};

	UPROPERTY(
		BlueprintAssignable,
		Category = "SingularisInteraction|引力奇点交互|事件分发器",
		meta = (DisplayName = "交互未悬浮时触发")
	)
	FOnInteractionUnhoverSignature OnInteractionUnhoverEvent{};

#pragma endregion

private:
#pragma region Internal Variable

	bool bIsEnabled = true;
	bool bIsHovered = false;

#pragma endregion

public:
#pragma region Constructors

	USingularisInteractionComponent();

#pragma endregion

#pragma region ActorComponent Interface

	virtual void BeginPlay() override;

#pragma endregion

#pragma region State

	UFUNCTION(
		BlueprintPure,
		BlueprintCallable,
		Category = "SingularisInteraction|引力奇点交互|State",
		meta = (DisplayName = "Enabled")
	)
	bool Enabled() const { return bIsEnabled; }

	UFUNCTION(
		BlueprintPure,
		BlueprintCallable,
		Category = "SingularisInteraction|引力奇点交互|State",
		meta = (DisplayName = "Hovered")
	)
	bool Hovered() const { return bIsHovered; }

#pragma endregion

#pragma region API

	UFUNCTION(
		BlueprintCallable,
		Category = "SingularisInteraction|引力奇点交互|API",
		meta = (DisplayName = "SetEnabled")
	)
	void SetEnabled(bool IsEnabled);

	UFUNCTION(
		BlueprintCallable,
		Category = "SingularisInteraction|引力奇点交互|API",
		meta = (DisplayName = "SetHovered")
	)
	void SetHovered(bool IsHovered);

#pragma endregion

#pragma region SPI

	UFUNCTION(
		BlueprintCallable,
		BlueprintAuthorityOnly,
		Category = "SingularisInteraction|引力奇点交互|SPI",
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

	void RegisterInteractionSubObjects();

#pragma endregion
};

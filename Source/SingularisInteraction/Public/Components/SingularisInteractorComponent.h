/* ====================================================================== *
 * SingularisInteractorComponent.h                                        *
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
#include <Components/ActorComponent.h>

#include "Objects/SingularisInteractionQueryer.h"
#include "Types/SingularisInteractorComponentType.h"
#include "SingularisInteractorComponent.generated.h"

class UInputAction;
class UInputMappingContext;
class USingularisInteractionQueryer;
class USingularisInteractionComponent;

#pragma region 委托签名

/** 交互者锁定的交互目标发生变化时广播 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnInteractionTargetChangedSignature,
	USingularisInteractionComponent*,
	OldTarget,
	USingularisInteractionComponent*,
	NewTarget
);

/** 交互者发起一次交互请求时广播 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnInteractionTriggeredSignature,
	USingularisInteractionComponent*,
	Target,
	FGameplayTag,
	StrategyTag
);

#pragma endregion

/**
 * 引力奇点交互者组件。
 *
 * 挂载于玩家控制器，逐帧执行视线查询锁定可交互目标，维护悬浮状态与输入映射上下文，
 * 并将本地交互输入经服务器 RPC 转交给目标交互组件处理。
 */
UCLASS(
	Blueprintable,
	BlueprintType,
	ClassGroup = ("Singularis"),
	meta = (BlueprintSpawnableComponent, DisplayName = "引力奇点交互者组件")
)
class SINGULARISINTERACTION_API USingularisInteractorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
#pragma region Parameter

	/** 引力奇点交互查询器 */
	UPROPERTY(
		Instanced,
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "引力奇点交互者组件",
		meta = (DisplayName = "引力奇点交互查询器")
	)
	USingularisInteractionQueryer* InteractionQueryer = nullptr;

	/** 输入优先级 */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "引力奇点交互者组件",
		meta = (DisplayName = "输入优先级")
	)
	int32 InputPriority = 10;

	/** 输入映射上下文 */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "引力奇点交互者组件",
		meta = (DisplayName = "输入映射上下文")
	)
	UInputMappingContext* InputMappingContext = nullptr;

	/** 交互者输入集 */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "引力奇点交互者组件",
		meta = (DisplayName = "交互者输入集")
	)
	TArray<FSingularisInteractorInput> InteractorInputs{};

#pragma endregion

#pragma region Event Dispatcher

	/** 交互者锁定的交互目标发生变化时广播 */
	UPROPERTY(
		BlueprintAssignable,
		Category = "引力奇点交互者组件|事件分发器",
		meta = (DisplayName = "交互目标变更时触发")
	)
	FOnInteractionTargetChangedSignature OnInteractionTargetChangedEvent{};

	/** 交互者发起一次交互请求时广播 */
	UPROPERTY(
		BlueprintAssignable,
		Category = "引力奇点交互者组件|事件分发器",
		meta = (DisplayName = "发起交互时触发")
	)
	FOnInteractionTriggeredSignature OnInteractionTriggeredEvent{};

#pragma endregion

private:
#pragma region State

	/** 拥有本组件的玩家控制器 */
	TWeakObjectPtr<APlayerController> OwnerPlayerController = nullptr;

	/** 当前视线查询结果 */
	FSingularisInteractionQueryerResult CurrentQueryerResult{};

#pragma endregion

public:
#pragma region Constructors

	USingularisInteractorComponent();

#pragma endregion

#pragma region ActorComponent Interface

	virtual void BeginPlay() override;
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction
	) override;

#pragma endregion

#pragma region API

	/**
	 * 当前锁定的交互目标组件。
	 *
	 * @return 当前目标组件；无有效目标时返回 nullptr。
	 */
	UFUNCTION(
		BlueprintPure,
		Category = "引力奇点交互者组件|API",
		meta = (DisplayName = "GetCurrentTarget")
	)
	USingularisInteractionComponent* GetCurrentTarget() const { return CurrentQueryerResult.InteractionComponent; }

#pragma endregion

private:
#pragma region RPC

	/** 服务器 RPC：将本地的交互请求与数据跨端发送给服务器 */
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RequestInteraction(
		USingularisInteractionComponent* TargetInteractionComponent,
		FGameplayTag StrategyTag,
		FInputActionValue ActionValue
	);

#pragma endregion

#pragma region Callback

	/** 本地交互输入触发的回调 */
	void HandleInteractionAction(const FInputActionValue& ActionValue, FGameplayTag StrategyTag);

#pragma endregion

#pragma region Internal Function

	/** 绑定交互者输入集中的增强输入动作 */
	void BindInput();

	/** 依据当前交互状态动态刷新输入映射上下文 */
	void RefreshInput() const;

	/** 执行一次视线查询并刷新当前交互状态 */
	void Query();

	/** 设置查询结果 */
	void SetQueryerResult(const FSingularisInteractionQueryerResult& QueryerResult);

	/** 响应式编程：应用查询结果副作用 */
	void ApplyQueryerResult(const FSingularisInteractionQueryerResult& OldQueryerResult) const;

#pragma endregion
};

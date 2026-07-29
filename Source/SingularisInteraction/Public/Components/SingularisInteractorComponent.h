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

	UPROPERTY(
		Instanced,
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "SingularisInteraction|引力奇点交互者|参数",
		meta = (DisplayName = "引力奇点交互查询器")
	)
	USingularisInteractionQueryer* InteractionQueryer = nullptr;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "SingularisInteraction|引力奇点交互者|输入",
		meta = (DisplayName = "输入优先级")
	)
	int32 InputPriority = 10;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "SingularisInteraction|引力奇点交互者|输入",
		meta = (DisplayName = "输入映射上下文")
	)
	UInputMappingContext* InputMappingContext = nullptr;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "SingularisInteraction|引力奇点交互者|输入",
		meta = (DisplayName = "交互者输入集")
	)
	TArray<FSingularisInteractorInput> InteractorInputs{};

#pragma endregion

private:
#pragma region Internal Variable

	TWeakObjectPtr<APlayerController> OwnerPlayerController = nullptr;

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

private:
#pragma region Network

	// Server RPC: 负责将本地的交互请求与数据跨端发送给服务器
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RequestInteraction(
		USingularisInteractionComponent* TargetInteractionComponent,
		FGameplayTag StrategyTag,
		FInputActionValue ActionValue
	);

#pragma endregion

#pragma region Internal Function

	void BindInputAction();
	void RefreshInputMappingContext() const;

	void Query();
	void SetCurrentQueryerResult(const FSingularisInteractionQueryerResult& QueryerResult);

#pragma endregion

#pragma region Callback

	// 本地输入触发的回调函数
	void HandleInteractionAction(const FInputActionValue& ActionValue, FGameplayTag StrategyTag);

#pragma endregion
};

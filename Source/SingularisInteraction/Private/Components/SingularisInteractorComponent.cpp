/* ====================================================================== *
 * SingularisInteractorComponent.cpp                                      *
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

#include "Components/SingularisInteractorComponent.h"

#include <EnhancedInputComponent.h>
#include <EnhancedInputSubsystems.h>
#include <InputMappingContext.h>
#include <Engine/LocalPlayer.h>
#include <GameFramework/Actor.h>
#include <GameFramework/Pawn.h>
#include <GameFramework/PlayerController.h>
#include <UObject/ConstructorHelpers.h>

#include "SingularisInteraction.h"
#include "Components/SingularisInteractionComponent.h"
#include "Objects/SingularisInteractionQueryer.h"
#include "Types/SingularisInteractionGameplayTags.h"

USingularisInteractorComponent::USingularisInteractorComponent()
{
	SetIsReplicatedByDefault(true);

	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.bCanEverTick = true;

	bAutoActivate = true;

	InteractionQueryer = CreateDefaultSubobject<USingularisInteractionQueryer>("InteractionQueryer");

	static const ConstructorHelpers::FObjectFinder<UInputMappingContext> DefaultInputMappingContextFinder(
		TEXT("/SingularisInteraction/Inputs/IMC_Default_SingularisInteractor.IMC_Default_SingularisInteractor")
	);
	static const ConstructorHelpers::FObjectFinder<UInputAction> DefaultInteractionActionFinder(
		TEXT("/SingularisInteraction/Inputs/Actions/IA_Interaction.IA_Interaction")
	);

	if (DefaultInputMappingContextFinder.Succeeded())
		InputMappingContext = DefaultInputMappingContextFinder.Object;
	else
		UE_LOG(
		LogSingularisInteraction,
		Error,
		TEXT("默认输入映射上下文加载失败：%s"),
		TEXT("/SingularisInteraction/Inputs/IMC_Default_SingularisInteractor")
	);

	if (DefaultInteractionActionFinder.Succeeded())
		InteractorInputs.Add({DefaultInteractionActionFinder.Object, SingularisInteraction_Strategy_Default});
	else
		UE_LOG(
		LogSingularisInteraction,
		Error,
		TEXT("默认交互输入动作加载失败：%s"),
		TEXT("/SingularisInteraction/Inputs/Actions/IA_Interaction")
	);
}

void USingularisInteractorComponent::BeginPlay()
{
	Super::BeginPlay();

	checkf(
		GetOwner()->IsA<APlayerController>(),
		TEXT("[%s] Owner 非 PlayerController"),
		*GetNameSafe(GetOwner())
	);

	// 1) 缓存拥有者玩家控制器
	OwnerPlayerController = Cast<APlayerController>(GetOwner());

	// 2) 绑定交互者输入集
	BindInput();

	// 3) 刷新输入映射上下文
	RefreshInput();

	UE_LOG(
		LogSingularisInteraction,
		Display,
		TEXT("[%s] BeginPlay：交互者组件初始化完成"),
		*GetNameSafe(GetOwner())
	);
}

void USingularisInteractorComponent::TickComponent(
	const float DeltaTime,
	const ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction
)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	Query();
}

bool USingularisInteractorComponent::Server_RequestInteraction_Validate(
	USingularisInteractionComponent* TargetInteractionComponent,
	FGameplayTag StrategyTag,
	FInputActionValue ActionValue
)
{
	// 1) 防作弊安全校验：确保目标组件在服务器端也依然有效
	if (!IsValid(TargetInteractionComponent))
	{
		UE_LOG(
			LogSingularisInteraction,
			Warning,
			TEXT("[%s] Server_RequestInteraction_Validate：目标交互组件无效，拒绝请求"),
			*GetNameSafe(GetOwner())
		);
		return false;
	}

	// 此处后续可补充网络距离校验防作弊（如：距离过远则拒绝交互）
	return true;
}

void USingularisInteractorComponent::Server_RequestInteraction_Implementation(
	USingularisInteractionComponent* TargetInteractionComponent,
	const FGameplayTag StrategyTag,
	const FInputActionValue ActionValue
)
{
	// 1) 二次校验并确保存在拥有者
	if (!IsValid(TargetInteractionComponent))
		return;
	if (!OwnerPlayerController.IsValid())
		return;

	// 2) 在服务器执行核心业务逻辑（此时调用的 BlueprintAuthorityOnly 业务逻辑即合法）
	TargetInteractionComponent->TryInteraction(
		StrategyTag,
		OwnerPlayerController.Get(),
		ActionValue
	);
}

void USingularisInteractorComponent::BindInput()
{
	// 1) 安全性检查：仅在拥有本地控制权时绑定输入
	if (!OwnerPlayerController.IsValid() || !OwnerPlayerController->IsLocalController())
		return;

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(
		OwnerPlayerController->InputComponent
	);
	if (!IsValid(EnhancedInputComponent))
	{
		UE_LOG(
			LogSingularisInteraction,
			Warning,
			TEXT("[%s] BindInput：EnhancedInputComponent 无效，交互输入不可用"),
			*GetNameSafe(GetOwner())
		);
		return;
	}

	// 2) 遍历并绑定交互者输入集
	for (const auto& [InputAction, Tag] : InteractorInputs)
	{
		if (!IsValid(InputAction) || !Tag.IsValid())
			continue;

		EnhancedInputComponent->BindAction(
			InputAction,
			ETriggerEvent::Started,
			this,
			&USingularisInteractorComponent::HandleInteractionAction,
			Tag
		);
	}
}

void USingularisInteractorComponent::RefreshInput() const
{
	// 1) 基础检查
	if (!OwnerPlayerController.IsValid() || !OwnerPlayerController->IsLocalController()) return;
	if (!IsValid(InputMappingContext)) return;

	UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(OwnerPlayerController->GetLocalPlayer());
	if (!IsValid(Subsystem))
	{
		UE_LOG(
			LogSingularisInteraction,
			Warning,
			TEXT("[%s] RefreshInput：EnhancedInput 本地玩家子系统无效"),
			*GetNameSafe(GetOwner())
		);
		return;
	}

	// 2) 根据当前交互状态动态添加或移除映射上下文
	if (CurrentQueryerResult.IsInteractionValid())
		Subsystem->AddMappingContext(InputMappingContext, InputPriority);
	else
		Subsystem->RemoveMappingContext(InputMappingContext);
}

void USingularisInteractorComponent::Query()
{
	if (!OwnerPlayerController.IsValid() || !OwnerPlayerController->IsLocalController()) return;
	if (!IsValid(InteractionQueryer)) return;

	// 1) 获取本地玩家视线数据
	FVector ViewLoc;
	FRotator ViewRot;
	OwnerPlayerController->GetPlayerViewPoint(ViewLoc, ViewRot);

	// 2) 构建查询交互参数
	FSingularisInteractionQueryerParams QueryerParams;
	QueryerParams.ViewLoc = ViewLoc;
	QueryerParams.ViewRot = ViewRot;
	QueryerParams.IgnoredActor = OwnerPlayerController->GetPawn();

	// 3) 执行查询交互并获取结果
	FSingularisInteractionQueryerResult QueryerResult;
	const bool bIsInteractionHit = InteractionQueryer->Query(QueryerResult, QueryerParams);

	// 4) 使用访问器模式更新当前交互状态
	SetQueryerResult(QueryerResult);
}

void USingularisInteractorComponent::SetQueryerResult(
	const FSingularisInteractionQueryerResult& QueryerResult
)
{
	// 1) 本地玩家检查
	if (!OwnerPlayerController.IsValid() || !OwnerPlayerController->IsLocalController()) return;

	// 2) 幂等性检查，若状态未变更则直接返回
	if (CurrentQueryerResult == QueryerResult) return;

	// 3) 捕获旧状态后写入新状态
	const FSingularisInteractionQueryerResult OldQueryerResult = CurrentQueryerResult;
	CurrentQueryerResult = QueryerResult;

	// 4) 响应式编程：应用查询结果副作用
	ApplyQueryerResult(OldQueryerResult);

	// 5) 广播交互目标变更
	OnInteractionTargetChangedEvent.Broadcast(
		OldQueryerResult.InteractionComponent,
		CurrentQueryerResult.InteractionComponent
	);

	// 6) 目标迁移属低频状态事件（非每帧），记录新旧目标便于追踪锁定链路
	UE_LOG(
		LogSingularisInteraction,
		Display,
		TEXT("[%s] SetQueryerResult：交互目标 %s → %s"),
		*GetNameSafe(GetOwner()),
		*GetNameSafe(OldQueryerResult.InteractionComponent),
		*GetNameSafe(CurrentQueryerResult.InteractionComponent)
	);
}

void USingularisInteractorComponent::ApplyQueryerResult(
	const FSingularisInteractionQueryerResult& OldQueryerResult
) const
{
	// 1) 清理旧状态：取消旧目标的悬停状态
	if (OldQueryerResult.IsInteractionValid())
		OldQueryerResult.InteractionComponent->SetHovered(false);

	// 2) 设置新状态：激活新目标的悬停状态
	if (CurrentQueryerResult.IsInteractionValid())
		CurrentQueryerResult.InteractionComponent->SetHovered(true);

	// 3) 根据最新状态刷新输入映射上下文
	RefreshInput();
}

// ReSharper disable CppMemberFunctionMayBeConst

void USingularisInteractorComponent::HandleInteractionAction(
	const FInputActionValue& ActionValue,
	const FGameplayTag StrategyTag
)
{
	// 1) 基础环境与目标校验
	if (!OwnerPlayerController.IsValid() || !OwnerPlayerController->IsLocalController()) return;
	if (!CurrentQueryerResult.IsInteractionValid())
	{
		UE_LOG(
			LogSingularisInteraction,
			Display,
			TEXT("[%s] HandleInteractionAction：无有效交互目标，忽略"),
			*GetNameSafe(GetOwner())
		);
		return;
	}

	// 2) 获取目标交互组件
	USingularisInteractionComponent* TargetInteractionComponent = CurrentQueryerResult.InteractionComponent;
	if (!IsValid(TargetInteractionComponent))
		return;

	// 3) 广播交互触发
	OnInteractionTriggeredEvent.Broadcast(TargetInteractionComponent, StrategyTag);

	// 4) 向服务器发起交互 RPC（过桥）
	Server_RequestInteraction(TargetInteractionComponent, StrategyTag, ActionValue);

	UE_LOG(
		LogSingularisInteraction,
		Display,
		TEXT("[%s] HandleInteractionAction：请求交互目标 %s，标签 %s"),
		*GetNameSafe(GetOwner()),
		*GetNameSafe(TargetInteractionComponent),
		*StrategyTag.ToString()
	);
}

// ReSharper restore CppMemberFunctionMayBeConst

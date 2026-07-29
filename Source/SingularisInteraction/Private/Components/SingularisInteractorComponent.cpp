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
	if (DefaultInteractionActionFinder.Succeeded())
		InteractorInputs.Add({DefaultInteractionActionFinder.Object, SingularisInteraction_Strategy_Default});
}

void USingularisInteractorComponent::BeginPlay()
{
	Super::BeginPlay();

	checkf(GetOwner()->IsA<APlayerController>(), TEXT("SingularisInteractorComponent: Owner is not PlayerController"));

	OwnerPlayerController = Cast<APlayerController>(GetOwner());

	BindInputAction();
	RefreshInputMappingContext();
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
	if (!IsValid(TargetInteractionComponent)) return false;

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
	if (!IsValid(TargetInteractionComponent)) return;
	if (!OwnerPlayerController.IsValid()) return;

	// 2) 在服务器执行核心业务逻辑（此时调用的 BlueprintAuthorityOnly 业务逻辑即合法）
	TargetInteractionComponent->TryInteraction(
		StrategyTag,
		OwnerPlayerController.Get(),
		ActionValue
	);
}

void USingularisInteractorComponent::BindInputAction()
{
	// 1) 安全性检查：仅在拥有本地控制权时绑定输入
	if (!OwnerPlayerController.IsValid() || !OwnerPlayerController->IsLocalController()) return;

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(
		OwnerPlayerController->InputComponent
	);
	if (!IsValid(EnhancedInputComponent)) return;

	// 2) 遍历并绑定交互者输入集
	for (const auto& [InputAction, Tag] : InteractorInputs)
	{
		if (!IsValid(InputAction) || !Tag.IsValid()) continue;

		EnhancedInputComponent->BindAction(
			InputAction,
			ETriggerEvent::Started,
			this,
			&USingularisInteractorComponent::HandleInteractionAction,
			Tag
		);
	}
}

void USingularisInteractorComponent::RefreshInputMappingContext() const
{
	// 1) 基础检查
	if (!OwnerPlayerController.IsValid() || !OwnerPlayerController->IsLocalController()) return;
	if (!IsValid(InputMappingContext)) return;

	UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(OwnerPlayerController->GetLocalPlayer());
	if (!IsValid(Subsystem)) return;

	// 2) 根据当前交互状态动态添加或移除映射上下文
	if (CurrentQueryerResult.IsInteractionValid())
	{
		Subsystem->AddMappingContext(InputMappingContext, InputPriority);
	}
	else
	{
		Subsystem->RemoveMappingContext(InputMappingContext);
	}
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
	SetCurrentQueryerResult(QueryerResult);
}

void USingularisInteractorComponent::SetCurrentQueryerResult(
	const FSingularisInteractionQueryerResult& QueryerResult
)
{
	// 1) 幂等性检查，若状态未变更则直接返回
	if (CurrentQueryerResult == QueryerResult) return;

	// 2) 清理旧状态：取消旧目标的悬停状态
	if (CurrentQueryerResult.IsInteractionValid())
		CurrentQueryerResult.InteractionComponent->SetHovered(false);

	// 3) 赋予新状态
	CurrentQueryerResult = QueryerResult;

	// 4) 设置新状态：激活新目标的悬停状态
	if (QueryerResult.IsInteractionValid())
		QueryerResult.InteractionComponent->SetHovered(true);

	// 5) 根据最新状态刷新输入映射上下文
	RefreshInputMappingContext();
}

// ReSharper disable CppMemberFunctionMayBeConst

void USingularisInteractorComponent::HandleInteractionAction(
	const FInputActionValue& ActionValue,
	const FGameplayTag StrategyTag
)
{
	// 1) 基础环境与目标校验
	if (!OwnerPlayerController.IsValid() || !OwnerPlayerController->IsLocalController()) return;
	if (!CurrentQueryerResult.IsInteractionValid()) return;

	// 2) 获取目标交互组件
	USingularisInteractionComponent* TargetInteractionComponent = CurrentQueryerResult.InteractionComponent;
	if (!IsValid(TargetInteractionComponent)) return;

	// 3) 核心变更：由本地调用改为向服务器发起 RPC（过桥）
	Server_RequestInteraction(TargetInteractionComponent, StrategyTag, ActionValue);
}

// ReSharper restore CppMemberFunctionMayBeConst

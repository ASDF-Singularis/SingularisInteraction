/* ====================================================================== *
 * SingularisInteractionComponent.cpp                                     *
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

#include "Components/SingularisInteractionComponent.h"

#include <Components/PrimitiveComponent.h>
#include <Engine/World.h>
#include <GameFramework/PlayerController.h>

#include "Objects/SingularisInteractionBehaviorStrategy.h"
#include "Objects/SingularisInteractionStrategy.h"
#include "Subsystems/SingularisInteractionSubsystem.h"
#include "Types/SingularisInteractionBehaviorStrategyType.h"
#include "Types/SingularisInteractionComponentType.h"
#include "Types/SingularisInteractionStrategyType.h"

#define ECC_INTERACTION ECC_GameTraceChannel1

USingularisInteractionComponent::USingularisInteractionComponent()
{
	SetIsReplicatedByDefault(true);
	bReplicateUsingRegisteredSubObjectList = true;

	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = false;

	bAutoActivate = true;
}

void USingularisInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	// 1) 登记策略子对象以参与复制
	RegisterInteractionSubObjects();

	// 2) 在映射子系统登记交互目标组件
	const UWorld* World = GetWorld();
	if (!IsValid(World)) return;

	USingularisInteractionSubsystem* MappingSubsystem =
		World->GetSubsystem<USingularisInteractionSubsystem>();
	if (!IsValid(MappingSubsystem)) return;

	for (auto Reference : TargetComponentReferences)
	{
		UPrimitiveComponent* TargetComponent = Cast<UPrimitiveComponent>(Reference.GetComponent(GetOwner()));
		if (!IsValid(TargetComponent)) continue;

		MappingSubsystem->RegisterMapping(TargetComponent, this);
	}
}

void USingularisInteractionComponent::SetEnabled(const bool IsEnabled)
{
	// 1) 幂等检查，状态未变更时直接返回
	if (bIsEnabled == IsEnabled) return;
	bIsEnabled = IsEnabled;

	// 2) 广播状态变更事件
	if (IsEnabled)
		OnInteractionEnableEvent.Broadcast();
	else
		OnInteractionDisableEvent.Broadcast();

	// 3) 驱动行为策略执行副作用
	FSingularisInteractionBehaviorStrategyContext Context;
	Context.InteractionActor = GetOwner();
	Context.InteractionComponent = this;

	for (const auto& [Name, Description,Strategy] : InteractionBehaviorStrategies)
	{
		if (IsEnabled)
			Strategy->Enabled(Context);
		else
			Strategy->Disabled(Context);
	}
}

void USingularisInteractionComponent::SetHovered(const bool IsHovered)
{
	// 1) 幂等检查，状态未变更时直接返回
	if (bIsHovered == IsHovered) return;
	bIsHovered = IsHovered;

	// 2) 广播状态变更事件
	if (IsHovered)
		OnInteractionHoverEvent.Broadcast();
	else
		OnInteractionUnhoverEvent.Broadcast();

	// 3) 驱动行为策略执行副作用
	FSingularisInteractionBehaviorStrategyContext Context;
	Context.InteractionActor = GetOwner();
	Context.InteractionComponent = this;

	for (const auto& [Name, Description,Strategy] : InteractionBehaviorStrategies)
	{
		if (IsHovered)
			Strategy->Hovered(Context);
		else
			Strategy->Unhovered(Context);
	}
}

void USingularisInteractionComponent::TryInteraction(
	const FGameplayTag& StrategyTag,
	APlayerController* PlayerController,
	const FInputActionValue& InputActionValue
)
{
	if (!IsValid(PlayerController)) return;

	OnInteractionEvent.Broadcast();

	// 1) 组装上下文 (Context)
	FSingularisInteractionStrategyContext Context;
	Context.Controller = PlayerController;
	Context.Instigator = PlayerController->GetPawn();
	Context.Avatar = GetOwner();
	Context.Target = GetOwner();
	Context.InteractionComponent = this;
	Context.InputValue = InputActionValue;

	// 2) 层级匹配
	for (const auto& [Tag, Entry] : InteractionStrategyPipelineMapping)
	{
		if (Tag.MatchesTag(StrategyTag))
		{
			for (const auto& StrategyPipeline : Entry.Strategies)
				StrategyPipeline.Strategy->Execute(Context);
		}
	}
}

void USingularisInteractionComponent::RegisterInteractionSubObjects()
{
	// 1) 服务器权威检查
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;

	// 2) 登记策略管线子对象
	for (auto& [Tag, Pipeline] : InteractionStrategyPipelineMapping)
	{
		for (const auto& Entry : Pipeline.Strategies)
		{
			if (!IsValid(Entry.Strategy)) continue;

			AddReplicatedSubObject(Entry.Strategy);
		}
	}

	// 3) 登记行为策略子对象
	for (const auto& Entry : InteractionBehaviorStrategies)
	{
		if (!IsValid(Entry.BehaviorStrategy)) continue;

		AddReplicatedSubObject(Entry.BehaviorStrategy);
	}
}
